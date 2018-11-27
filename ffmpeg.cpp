
#include "ffmpeg.h"

#include <exception>
#include <unistd.h>

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

void FFmpegThread::StartFFmpeg(int channel)
{
	if (targetChannel != channel) {
		if ( Active() ) Stop();
	}
	targetChannel = channel;
	Start();
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
	cPipe pp;
	dsyslog("Live: FFmpegTread::Action() started channel = %d", targetChannel);
	try {
		int retry = 0;
		int count = 0;
		do {
			stringstream ss;
			ss << "mkdir -p /tmp/live-hls-buffer && "
				"cd /tmp/live-hls-buffer && "
				"rm -rf * && "
				"/tmp/ffmpeg -loglevel warning "
				"-analyzeduration 2M -probesize 1M "
				"-i \"http://localhost:3000/";
			ss << targetChannel;
			ss << "\" "
				"-map 0:v -map 0:a:0 -c:v copy -c:a aac "
				"-f hls -hls_time 1 -hls_start_number_source datetime -hls_allow_cache 0 -hls_flags delete_segments "
				"-master_pl_name master_";
			ss << targetChannel;
			ss << ".m3u8 ffmpeg_";
			ss << targetChannel;
			ss << "_data.m3u8";
			bool ret = pp.Open(ss.str().c_str(), "w"); // start ffmpeg

			dsyslog("Live: FFmpegTread::Action::Open(%d) ffmpeg started", ret);
			
			ss.str("");
			ss << "/tmp/live-hls-buffer/master_";
			ss << targetChannel;
			ss << ".m3u8";

			count = 0;
			do {
				cw.Wait(1000);
				ifstream f(ss.str().c_str());
				if (f.good()) break;
				dsyslog("Live: FFmpegTread::Action() ffmpeg starting... %d", count);
			} while (Running() && ++count < 10);
			dsyslog("Live: FFmpegTread::Action() ffmpeg running %d", count);

			if (count < 10) break;
			else {
				fwrite("q", 1, 1, pp); fflush(pp); // send quit commmand to ffmpeg
				usleep(500e3);
				int r = pp.Close();
				dsyslog("Live: FFmpegTread::Action::Close(%d) disabled ffmpeg", r);
			}
		} while (retry++ < 1 && Running());
		if (retry > 1) return;

		touch = false;
		count = 0;
		while (Running() && count++ < 20) {
			if (touch) {
				touch = false;
				count = 0;
			}
			if (count) dsyslog("Live: FFmpegTread::Action() waiting: %2d/20", count);
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

} // namespace vdrlive
