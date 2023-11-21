#include <MainTest.h>

static Main_Test Test;

extern "C" void app_main(void)
{
    Test.setup();

    Test.loop();
}