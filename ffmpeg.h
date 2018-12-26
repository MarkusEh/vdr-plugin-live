#ifndef VDR_LIVE_THREAD_H
#define VDR_LIVE_THREAD_H

#include <vdr/thread.h>

namespace vdrlive {

class FFmpegThread : public cThread {
public:
	FFmpegThread();
	~FFmpegThread();

	void StartFFmpeg(int channel, int vopt);
	void Stop();
	void Touch();

protected:
	void Action();

private:
	cCondWait cw;
	bool touch;
	int targetChannel;
	int vOption;
};

// cPipe2 implements a pipe that closes all unnecessary file descriptors in
// the child process. This is an improved variant of the vdr::cPipe

class cPipe2 {
private:
	pid_t pid;
	FILE *f;
public:
	cPipe2(void);
	~cPipe2();
	operator FILE* () { return f; }
	bool Open(const char *Command, const char *Mode);
	int Close(void);
};

} // namespace vdrlive

#endif // VDR_LIVE_THREAD_H
