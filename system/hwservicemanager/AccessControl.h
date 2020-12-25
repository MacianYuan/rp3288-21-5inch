#include <string>

#include <selinux/android.h>
#include <selinux/avc.h>

namespace android {

class AccessControl {
public:
    AccessControl();

    struct CallingContext {
        bool sidPresent;
        std::string sid;
        pid_t pid;
    };
    static CallingContext getCallingContext(pid_t sourcePid);

    bool canAdd(const std::string& fqName, const CallingContext& callingContext);
    bool canGet(const std::string& fqName, const CallingContext& callingContext);
    bool canList(const CallingContext& callingContext);

private:

    bool checkPermission(const CallingContext& source, const char *targetContext, const char *perm, const char *interface);
    bool checkPermission(const CallingContext& source, const char *perm, const char *interface);

    static int auditCallback(void *data, security_class_t cls, char *buf, size_t len);

    char*                  mSeContext;
    struct selabel_handle* mSeHandle;
    union selinux_callback mSeCallbacks;
};

} // namespace android
