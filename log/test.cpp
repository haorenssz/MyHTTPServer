#include "log.h"

void TestLog() {
    int cnt = 0, level = 0;
    int m_close_log = 0;
    Log::get_instance()->init("./testlog1", m_close_log, 2000, 800000, 800);

    LOG_ERROR("%s","error test!");
    LOG_DEBUG("%s","debug test!");

    /*
    for(level = 3; level >= 0; level--) {
        //Log::get_instance()->write_log()
        for(int j = 0; j < 10000; j++ ){
            for(int i = 0; i < 4; i++) {
                LOG_BASE(i,"%s 111111111 %d ============= ", "Test", cnt++);
            }
        }
    }
    cnt = 0;
    Log::Instance()->init(level, "./testlog2", ".log", 5000);
    for(level = 0; level < 4; level++) {
        Log::Instance()->SetLevel(level);
        for(int j = 0; j < 10000; j++ ){
            for(int i = 0; i < 4; i++) {
                LOG_BASE(i,"%s 222222222 %d ============= ", "Test", cnt++);
            }
        }
    }
    */
}

int main()
{
    //TestLog();
    int cnt = 0, level = 0;
    int m_close_log = 0;
    Log::get_instance()->init("./testlog1", m_close_log, 2000, 800000, 800);

    LOG_ERROR("%s","error test!");
    LOG_DEBUG("%s","debug test!");
    return 0;
}