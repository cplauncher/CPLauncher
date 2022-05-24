#ifndef PLATFORM_NATIVE_H
#define PLATFORM_NATIVE_H
extern "C" {
    void deactivateApp();
    void activateApp();
    const char* defaultFolderPath();
}
#endif // PLATFORM_NATIVE_H
