#ifndef VDR_LIVE_THREAD_H
#define VDR_LIVE_THREAD_H

#include <vdr/thread.h>
#include <string>

namespace vdrlive {

class FFmpegThread : public cThread {
public:
	FFmpegThread();
	~FFmpegThread();

	void StartFFmpeg(std::string s, int channel, int vopt);
	void Stop();
	void Touch();

protected:
	void Action();

private:
	cCondWait cw;
	bool touch = false;
	int targetChannel;
	int vOption = 0;
	std::string session;
};

// cPipe2 implements a pipe that closes all unnecessary file descriptors in
// the child process. This is an improved variant of the vdr::cPipe

class cPipe2 {
private:
	pid_t pid;
	bool terminated = false;
	FILE *f;
public:
	cPipe2(void);
	~cPipe2();
	operator FILE* () { return f; }
	bool Open(const char *Command, const char *Mode);
	int Check(void);
	int Close(void);
};

} // namespace vdrlive

#endif // VDR_LIVE_THREAD_H
