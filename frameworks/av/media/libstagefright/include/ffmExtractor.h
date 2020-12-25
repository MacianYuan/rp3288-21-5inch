#ifndef __FFMEXTRACTOR_H__
#define __FFMEXTRACTOR_H__

namespace android {

class DataSource;
class MediaExtractor;
class String8;

static const char* FFMPEG_DEMUX_LIB = "libffmpeg_utils.so";
static const char* CREATE_FFMPEG_EXTRACTOR = "CreateFFMPEGExtractor";
static const char* FFMPEG_SNIFFER = "SniffFFMPEG";
typedef void * ffmpegExtractorFactory(const void *mime, const void *meta, const void *source);

typedef bool SniffFunc(const void *source, void *mimeType, void *confidence, void *meta);

class ffmExtractor
{
public:
    static sp<MediaExtractor> CreateExtractor(const sp<DataSource> &source, const char* mime, const sp<AMessage> &meta);
    static bool SniffFFMPEG(const sp<DataSource> &source, String8 *mimeType, float *confidence, sp<AMessage> *meta);
};

}  // namespace android

#endif //__FFMEXTRACTOR_H__

