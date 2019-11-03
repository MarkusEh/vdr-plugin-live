
#include "ffmpeg.h"
#include "setup.h"

#include <exception>
#include <unistd.h>
#include <sys/wait.h>

#include <vdr/tools.h>
#include <tnt/tntnet.h>

namespace vdrlive {

using namespace std;


FFmpegThread::FFmpegThread()
	:cThread("stream utility handler")
{
	targetChannel = -1;
	dsyslog("Live: FFmpegTread() created");
}

FFmpegThread::~FFmpegThread()
{
	Stop();
	dsyslog("Live: FFmpegTread() destructed");
}

void FFmpegThread::StartFFmpeg(std::string s, int channel, int vopt)
{
	if (targetChannel != channel || vOption != vopt) {
		dsyslog("Live: FFmpegTread::StartFFmpeg() change channel %d -> %d", targetChannel, channel);
		if ( Active() ) Stop();
		targetChannel = channel;
		vOption = vopt;
	}
	session = s;
	Start();
	dsyslog("Live: FFmpegTread::StartFFmpeg() completed");
}

void FFmpegThread::Stop()
{
	cw.Signal();
	dsyslog("Live: FFmpegTread::Stop() try stopping");
	if ( Active() ) Cancel( 5 );
	dsyslog("Live: FFmpegTread::Stop() stopped");
}

void FFmpegThread::Touch()
{
	touch = true;
}

void FFmpegThread::Action()
{
	dsyslog("Live: FFmpegTread::Action() started channel = %d", targetChannel);

	cPipe2 pp;

	std::string def = "ffmpeg -loglevel warning -f mpegts -analyzeduration 1.2M -probesize 5M "
		"-i <input> -map 0:v -map 0:a:0 -c:v copy -c:a aac -ac 2";
	vector<string> vopts;
	vopts.push_back( LiveSetup().GetStreamVideoOpt0() );
	vopts.push_back( LiveSetup().GetStreamVideoOpt1() );
	vopts.push_back( LiveSetup().GetStreamVideoOpt2() );
	vopts.push_back( LiveSetup().GetStreamVideoOpt3() );

	if (vopts[0].empty() || vopts[0].find("<input>") == string::npos) { // h264
		vopts[0] = def;
		LiveSetup().SetStreamVideoOpt0(vopts[0]);
	}
	if (vopts[1].empty() || vopts[1].find("<input>") == string::npos) { // h265
		vopts[1] = def;
		LiveSetup().SetStreamVideoOpt1(vopts[1]);
	}
	if (vopts[2].empty() || vopts[2].find("<input>") == string::npos) { // mpeg2
		vopts[2] = def;
		LiveSetup().SetStreamVideoOpt2(vopts[2]);
	}
	if (vopts[3].empty() || vopts[3].find("<input>") == string::npos) { // others
		vopts[3] = def;
		LiveSetup().SetStreamVideoOpt3(vopts[3]);
	}

	string packerCmd(vopts[vOption]);
	stringstream ss;
	ss.str("");
	ss << "\"http://localhost:" << LiveSetup().GetStreamdevPort() << "/" << targetChannel << "\"";
	packerCmd.replace(packerCmd.find("<input>"), 7, ss.str());
	dsyslog("Live: FFmpegTread::Action packetizer cmd: %s", packerCmd.c_str());

	try {
		int retry = 0;
		int count = 0;
		do {
			ss.str("");
			ss << "mkdir -p /tmp/live-hls-buffer/" << session << " && "
				"cd /tmp/live-hls-buffer/" << session << " && rm -rf * && "
				"exec " << packerCmd << " "
				"-f hls -hls_time 1 -hls_start_number_source datetime -hls_flags delete_segments "
				"-master_pl_name master_";
			ss << targetChannel;
			ss << ".m3u8 ffmpeg_";
			ss << targetChannel;
			ss << "_data.m3u8";
			bool ret = pp.Open(ss.str().c_str(), "w"); // start ffmpeg

			dsyslog("Live: FFmpegTread::Action::Open(%d) ffmpeg started", ret);

			ss.str("");
			ss << "/tmp/live-hls-buffer/" << session << "/master_";
			ss << targetChannel;
			ss << ".m3u8";

			count = 0;
			do {
				cw.Wait(1000);
				ifstream f(ss.str().c_str());
				if (f.good()) break; // check if ffmpeg starts to generate output
				dsyslog("Live: FFmpegTread::Action() ffmpeg starting... %d", count);
			} while (Running() && pp.Check() == 0 && ++count < 6);
			if (pp.Check() < 0) continue;

			if (count < 6) {
				dsyslog("Live: FFmpegTread::Action() ffmpeg running %d", count);
				break;
			}
			else {  // ffmpeg did not start properly
				fwrite("q", 1, 1, pp); fflush(pp); // send quit commmand to ffmpeg
				usleep(200e3);
				int r = pp.Close();
				dsyslog("Live: FFmpegTread::Action::Close(%d) disabled ffmpeg", r);
				usleep(500e3);
			}
		} while (retry++ < 2 && Running());
		if (retry > 1) return;

		touch = false;
		count = 0;
		while (Running() && pp.Check() == 0 && count++ < 60) {
			if (touch) {
				touch = false;
				count = 0;
			}
			cw.Wait(1000);
		}
		fwrite("q", 1, 1, pp); fflush(pp); // send quit commmand to ffmpeg
		usleep(500e3);
		int r = pp.Close();
		dsyslog("Live: FFmpegTread::Action::Close(%d) disabled ffmpeg", r);

	} catch (exception const& ex) {
		esyslog("ERROR: live FFmpegTread::Action() failed: %s", ex.what());
	}
	dsyslog("Live: FFmpegTread::Action() finished");
}

// --- cPipe2 -----------------------------------------------------------------
 
// cPipe2::Open() and cPipe2::Close() are based on code originally received from
// Andreas Vitting <Andreas@huji.de>
 
cPipe2::cPipe2(void)
{
	pid = -1;
	f = NULL;
}
 
cPipe2::~cPipe2()
{
	Close();
}

bool cPipe2::Open(const char *Command, const char *Mode)
{
	int fd[2];

	if (pipe(fd) < 0) {
		LOG_ERROR;
		return false;
	}
	if ((pid = fork()) < 0) { // fork failed
		LOG_ERROR;
		close(fd[0]);
		close(fd[1]);
		return false;
	}

	const char *mode = "w";
	int iopipe = 0;

	if (pid > 0) { // parent process
		terminated = false;
		if (strcmp(Mode, "r") == 0) {
			mode = "r";
			iopipe = 1;
		}
		close(fd[iopipe]);
		if ((f = fdopen(fd[1 - iopipe], mode)) == NULL) {
			LOG_ERROR;
			close(fd[1 - iopipe]);
		}
		return f != NULL;
	}
	else { // child process
		int iofd = STDOUT_FILENO;
		if (strcmp(Mode, "w") == 0) {
			iopipe = 1;
			iofd = STDIN_FILENO;
		}
		close(fd[iopipe]);
		if (dup2(fd[1 - iopipe], iofd) == -1) { // now redirect
			LOG_ERROR;
			close(fd[1 - iopipe]);
			_exit(-1);
		}
		else {
			int MaxPossibleFileDescriptors = getdtablesize();
			for (int i = STDERR_FILENO + 1; i < MaxPossibleFileDescriptors; i++)
				close(i); //close all dup'ed filedescriptors
			if (execl("/bin/sh", "sh", "-c", Command, NULL) == -1) {
				LOG_ERROR_STR(Command);
				close(fd[1 - iopipe]);
				_exit(-1);
			}
		}
		_exit(0);
	}
}

int cPipe2::Check(void)
{
	int ret = -1;

	if (terminated) return -1;
	if (pid > 0) {
		int status = 0;
		ret = waitpid(pid, &status, WNOHANG);;
		if (ret < 0) {
			if (errno != EINTR && errno != ECHILD) {
				LOG_ERROR;
				return ret;
			}
		}
		if (ret > 0) terminated = true;

	}

	return ret;
}

int cPipe2::Close(void)
{
	int ret = -1;

	if (f) {
		fclose(f);
		f = NULL;
	}

	if (pid > 0) {
		int status = 0;
		int i = 5;
		while (i > 0) {
			ret = waitpid(pid, &status, WNOHANG);
			if (ret < 0) {
				if (errno != EINTR && errno != ECHILD) {
					LOG_ERROR;
					break;
				}
			}
			else if (ret == pid)
				break;
			i--;
			cCondWait::SleepMs(100);
		}
		if (!i) {
			kill(pid, SIGINT);
			cCondWait::SleepMs(100);
			ret = waitpid(pid, &status, WNOHANG);
			kill(pid, SIGKILL);
			cCondWait::SleepMs(100);
			ret = waitpid(pid, &status, WNOHANG);
		}
		else if (ret == -1 || !WIFEXITED(status))
			ret = -1;
		pid = -1;
	}

	return ret;
}

} // namespace vdrlive
