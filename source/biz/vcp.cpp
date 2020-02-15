#include <stdio.h>
#include <string.h>

#include "cpp_os.h"
#include "bsp_vcp.h"
#include "misc_macro.h"
#include "sett_def.h"
#include "can.h"

#ifdef __cplusplus
    using namespace std;
#endif

#define IFACE1    "CAN1"
#define IFACE2    "CAN2"
#define IFACE3    "CAN3"
#define IFACE_ALL "ALL"

/// Cmd structure
typedef struct
{
    const char * const cmd;
    void (* const func)(char *&_str);

    /// Compare cmd and run function of command
    char * parse(char *&_str)
    {
        const uint8_t len = strlen(cmd);

        if (   false
            || strlen(_str) < len
            || memcmp(_str, cmd, len) != 0
            || (_str[len] != '\0' && _str[len] != ' ')
            )
        {
            return nullptr;
        }

        char *result = &_str[len];
        for(; *result == ' '; result++);

        if (func != nullptr)
        {
            func(result);
        }

        _str = result;
        return result;
    }
} cmd_t;

void cmd_set_can(char *&_str, const can_t _can)
{
    uint32_t baud = 0;
    PARAM_TRIGGER_TYPE_t loop_flag = PARAM_TRIGGER_NA;
    PARAM_TRIGGER_TYPE_t silent_flag = PARAM_TRIGGER_NA;
    char flags[] = "nn";
    uint32_t len_str = 0;

    sscanf(_str, " %u %n%[ls] %n", &baud, &len_str, flags, &len_str);
    _str = &_str[len_str];
    loop_flag   = (flags[0] == 'l' || flags[1] == 'l') ? PARAM_TRIGGER_ON : PARAM_TRIGGER_OFF;
    silent_flag = (flags[0] == 's' || flags[1] == 's') ? PARAM_TRIGGER_ON : PARAM_TRIGGER_OFF;

    const can_sett_t &can = sett_usr_param->can_sett[_can];
    sett_usr.set((PARAM_ID_TYPE_t)can.baud.header.id, baud);
    sett_usr.set((PARAM_ID_TYPE_t)can.loop.header.id, loop_flag);
    sett_usr.set((PARAM_ID_TYPE_t)can.slnt.header.id, silent_flag);

    fprintf(stderr, "<vcp> CAN%d parameters writed.\r\n", _can + 1);
}

void msg_send(char *&_str, const can_t _can)
{
    uint32_t id = 0;
    uint32_t len = 0;
    uint32_t len_str = 0;
    sscanf(_str, "%i %1u %n", &id, &len, &len_str);
    if (len_str == 0 || id == 0 || len == 0 || len > 8) return;
    _str = &_str[len_str];

    can_tx_msg_t msg;
    msg.id        = id ;
    msg.len       = len;
    msg.timestamp = 0  ;

    // Расшифровываем данные CAN-сообщения, представленные побайтно в виде hex строк без префикса 0x и разделенных пробелами
    for (uint8_t i = 0; i < len; i++)
    {
        char tmp[] = "0x00";
        len_str = 0;
        sscanf(_str, " %2[0-9A-Fa-f] %n", &tmp[2], &len_str);
        if (len_str == 0) return;
        _str = &_str[len_str];
        uint32_t tmp_data = 0;
        sscanf(tmp, "%i", &tmp_data);
        msg.data[i] = tmp_data;
    }

    uint32_t time = 0, cnt = 0;
    sscanf(_str, " %u%n|%n%u %n", &time, &len_str, &len_str, &cnt, &len_str);
    if (len_str == 0) return;
    _str = &_str[len_str];

    msg.interval   = time;
    msg.repeat_cnt = cnt;

    can_threads[_can].put(msg);
}

void cmd_set (char *&_str)
{
    static cmd_t cmd[] =
    {
        {IFACE1, [](char *&_str){cmd_set_can(_str, can_1);}},
        {IFACE2, [](char *&_str){cmd_set_can(_str, can_2);}},
        {IFACE3, [](char *&_str){cmd_set_can(_str, can_3);}},
    };

    for (uint8_t i = 0; i < countof(cmd); i++) cmd[i].parse(_str);
}

void cmd_info (char *&_str)
{
    for (uint8_t i = 0; i < countof(can_threads); i++)
    {
        const can_sett_t &can = sett_usr_param->can_sett[i];
        printf("CAN%d info:\r\n", i + 1);
        printf("  baudrate: %d bit/s;\r\n", can.baud.data);
        printf("  SJW: %d quants;\r\n", can.sjw.data);
        printf("  loop mode: %s\r\n",   (can.loop.header.state == PARAM_TRIGGER_ON) ? "on" : "off");
        printf("  silent mode: %s\r\n", (can.slnt.header.state == PARAM_TRIGGER_ON) ? "on" : "off");
        printf("  terminator: %s\r\n",  (can.term.header.state == PARAM_TRIGGER_ON) ? "on" : "off");
        printf("  channel status: %s\r\n\r\n", 
               (can_threads[i].get_state() == cpp_os::thread_error) ? "disconnect" : "connect");
    }
}

void ch_on(const uint8_t flags)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        if (flags & TO_FLAG(i))
        {
            can_threads[i].run();
            fprintf(stderr, "<vcp> CAN%d started.\r\n", i + 1);
        }
    }
}

void ch_off(const uint8_t flags)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        if (flags & TO_FLAG(i))
        {
            can_threads[i].terminate();
            fprintf(stderr, "<vcp> CAN%d stopped.\r\n", i + 1);
        }
    }
}

void ch_clr(const uint8_t flags)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        if (flags & TO_FLAG(i))
        {
            can_threads[i].reset();
            fprintf(stderr, "<vcp> CAN%d queue cleared.\r\n", i);
        }
    }
}

void cmd_start (char *&_str)
{
    static cmd_t cmd[] =
    {
        {IFACE1   , [](char *&_str){cmd_set_can(_str, can_1);}},
        {IFACE2   , [](char *&_str){cmd_set_can(_str, can_2);}},
        {IFACE3   , [](char *&_str){cmd_set_can(_str, can_3);}},
        {IFACE_ALL, [](char *&_str){(void)_str; ch_on(7);}},
    };

    uint8_t on_flags = 0;

    for (uint8_t i = 0; i < countof(cmd); i++)
    {
        if (cmd[i].parse(_str) != nullptr) on_flags |= 1 << i;
    }

    ch_on(on_flags);
}

void cmd_stop (char *&_str)
{
    static cmd_t cmd[] =
    {
        {IFACE1   , [](char *&_str){(void)_str; ch_off(TO_FLAG(can_1));}},
        {IFACE2   , [](char *&_str){(void)_str; ch_off(TO_FLAG(can_2));}},
        {IFACE3   , [](char *&_str){(void)_str; ch_off(TO_FLAG(can_3));}},
        {IFACE_ALL, [](char *&_str){(void)_str; ch_off(7);}},
    };

    for (uint8_t i = 0; i < countof(cmd); i++) cmd[i].parse(_str);
}

void cmd_send (char *&_str)
{
    static cmd_t cmd[] =
    {
        {IFACE1, [](char *&_str){msg_send(_str, can_1);}},
        {IFACE2, [](char *&_str){msg_send(_str, can_2);}},
        {IFACE3, [](char *&_str){msg_send(_str, can_3);}},
    };

    for (uint8_t i = 0; i < countof(cmd); i++) cmd[i].parse(_str);
}

void cmd_clear (char *&_str)
{
    static cmd_t cmd[] =
    {
        {IFACE1   , [](char *&_str){(void)_str; ch_clr(TO_FLAG(can_1));}},
        {IFACE2   , [](char *&_str){(void)_str; ch_clr(TO_FLAG(can_2));}},
        {IFACE3   , [](char *&_str){(void)_str; ch_clr(TO_FLAG(can_3));}},
        {IFACE_ALL, [](char *&_str){(void)_str; ch_clr(7);}},
    };

    for (uint8_t i = 0; i < countof(cmd); i++) cmd[i].parse(_str);
}


class : public cpp_os_timer
{
private:
    bool last_result = false;

    void timer_func()
    {
        bool result = bsp_vcp_is_configured();
    
        if (!result)
        {
            if (last_result)
            {
                fprintf(stderr, "<vcp> USB VCP stopped\r\n");
                
                bsp_vcp_deinit();
            }
            else
            {
                bsp_vcp_init();
            }
        }
        else if (!last_result)
        {
            fprintf(stderr, "<vcp> USB VCP runing\r\n");
            fprintf(stderr, "If you use Putty, you must select this settings:\r\n"
                "  - in bookmark 'connection/serial' disable 'flow control';\r\n"
                "  - in bookmark 'Terminal' check 'implict CR in every LF',\r\n"
                "    set 'Force on' in 'Local echo' and 'Local line ending'.\r\n"
            );

        }
        
        last_result = result;
    };
    
public:
    using cpp_os_timer::cpp_os_timer;
} vcp = {1000, true, "vcp"};

class : public cpp_os_thread<>
{
private:
    void thread_func(void)
    {
        vcp.start();

        for(;;)
        {
            static cmd_t cmd[] =
            {
                {"set"  , cmd_set  }, // Установить настройки для каналов
                {"info" , cmd_info }, // Получить настройки и информацию о системе
                {"start", cmd_start}, // Запуск сканирования
                {"stop" , cmd_stop }, // Ввыключение сканирования
                {"send" , cmd_send }, // Отправка сообщения
                {"clear", cmd_clear}, // Очистить очередь сообщений
            };

            static char buf[256];

            memset(buf, 0, sizeof(buf));

            while (gets(buf) != buf);

            uint16_t len = strlen(buf);

            if (len < 1 || len >= sizeof(buf)) continue;

            char *ch = buf;
            for (uint8_t i = 0; (cmd[i].parse(ch) == nullptr && i < countof(cmd)); i++);
        }
    }

public:
    using cpp_os_thread::cpp_os_thread;
} console = {true, cpp_os::priority_below_normal, "console"};
