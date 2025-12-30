#include "CameraWidget.h"
#include "CameraUI.h"

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    CameraWidget camera;
    camera.set_device("/dev/video0");
    camera.init();   // 初始化但不启动

    CameraUI ui(&camera);
    ui.build();
    ui.show();

    gtk_main();
    camera.stop();
    return 0;
}

