#ifndef RK_METADATA_RETRIEVER_H
#define RK_METADATA_RETRIEVER_H

#include <media/MediaMetadataRetrieverInterface.h>
#include <utils/KeyedVector.h>

#ifdef AVS50
#include <media/IMediaHTTPService.h>
#include <media/stagefright/MediaSource.h>
#endif

namespace android
{

class RetrieverDelegate;

class RK_MetadataRetriever: public MediaMetadataRetrieverInterface
{
public:
    RK_MetadataRetriever();
    virtual ~RK_MetadataRetriever();
#ifdef AVS50
    virtual status_t            setDataSource(const sp<IMediaHTTPService> &httpService,
                                              const char *uri, const KeyedVector<String8, String8> *headers);
    virtual status_t            setDataSource(const sp<DataSource>& source, const char *mime){ return NO_ERROR; };
#else
    virtual status_t            setDataSource(const char *url, const KeyedVector<String8, String8> *headers);
#endif
    virtual status_t            setDataSource(int fd, int64_t offset, int64_t length);
    virtual VideoFrame*         getFrameAtTime(int64_t timeUs, int option, int colorFormat, bool metaOnly);
    virtual MediaAlbumArt*      extractAlbumArt();
    virtual const char*         extractMetadata(int keyCode);
private:
    RetrieverDelegate *mRetrieverDelegate;
    RK_MetadataRetriever(const RK_MetadataRetriever &);
    RK_MetadataRetriever &operator=(const RK_MetadataRetriever &);
};
}
#endif // RK_METADATA_RETRIEVER_H
