#ifndef VDR_LIVE_THREAD_H
#define VDR_LIVE_THREAD_H

#include <atomic>

#include <vdr/thread.h>

namespace vdrlive {

class FFmpegThread : public cThread {
public:
	FFmpegThread();
	~FFmpegThread();

	void StartFFmpeg(int channel);
	void Stop();
	void Touch();

protected:
	void Action();

private:
	cCondWait cw;
	bool touch;
	int targetChannel;
};

} // namespace vdrlive

#endif // VDR_LIVE_THREAD_H
