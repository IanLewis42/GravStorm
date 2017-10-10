#include <jni.h>
#include <string>

extern "C"
JNIEXPORT jstring JNICALL
Java_com_tootiredgames_gravstorm_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

#include <allegro5/allegro5.h>

extern "C"
{
int game(int argc, char **argv );
}

int main(int argc, char **argv) {
	game (argc, argv);
	return 0;
}
