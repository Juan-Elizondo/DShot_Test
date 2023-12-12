#include <MainTest.h>
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
void IRAM_ATTR Main_Loop_Timer_Callback(void* params)
{
    bool* loop_timer = (bool*)params;
    *loop_timer = true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
void Main_Test::setup(void)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
void Main_Test::loop(void)
{
    dshot_frequency_t ESC_freq = DSHOT1200;
    bool bidirectional = true;
    bool TXS_Buffer = true;

    //ESCData ESC_L_data;
    ESCData ESC_R_data;
    //DShot_ESC L_ESC(ESC_freq, bidirectional, ESC_L, TXS_Buffer, "L_ESC", ESC_L_data);
    DShot_ESC R_ESC(ESC_freq, bidirectional, ESC_R, TXS_Buffer, "R_ESC", ESC_R_data);

    esp_timer_handle_t main_loop_timer_handle;
    bool main_loop_timer = false;
    const esp_timer_create_args_t main_loop_timer_args = {
        .callback = &Main_Loop_Timer_Callback,
        .arg = &main_loop_timer,
        .dispatch_method = ESP_TIMER_ISR,
        .name = "Main_Loop_Timer",
        .skip_unhandled_events = true
    };
    uint64_t period_us = 400;
    ESP_ERROR_CHECK(esp_timer_create(&main_loop_timer_args,&main_loop_timer_handle));
    esp_timer_start_periodic(main_loop_timer_handle, period_us);

    uint16_t throttle = 100;
    ESC_R_data.data_valid = 0;
    int counter = 0;
    while(true)
    {
        if(main_loop_timer)
        {
            main_loop_timer = false;
            //L_ESC.Throttle_Write(throttle);
            R_ESC.Throttle_Write(throttle);
        }
        if(ESC_R_data.data_valid)
        {
            
            ESC_R_data.data_valid = 0;
            counter++;
        }
        if(counter==1000000/period_us)
        {
            ESP_LOGI("Main", "eRPM: %u",(unsigned int)ESC_R_data.latest_eRPM);
            counter = 0;
        }
    }
}