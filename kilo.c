/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** data ***/

struct termios users_termios_settings;

/*** terminal ***/

void die(const char *message)
{
    perror(message);
    exit(1);
}

void disableRawMod()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &users_termios_settings) == -1)
        die("tcsetattr while disabling raw mode");
}

void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &users_termios_settings) == -1)
        die("tcgetattr");

    atexit(disableRawMod);

    struct termios raw_mode_settings = users_termios_settings;

    raw_mode_settings.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);          // for "local" flags
    raw_mode_settings.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); // for "input" flags
    raw_mode_settings.c_oflag &= ~(OPOST);                                  // "output" flags
    raw_mode_settings.c_cflag |= (CS8);                                     // for "control" flags)

    raw_mode_settings.c_cc[VMIN] = 0;
    raw_mode_settings.c_cc[VTIME] = 1; // 1/10 of a second or 100 milliseconds

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_mode_settings) == -1)
        die("tcsetattr while enabling raw mode");
}

/*** init ***/

int main()
{
    enableRawMode();

    while (1)
    {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");

        if (iscntrl(c))
        {
            printf("Control: %d\r\n", c);
        }
        else
        {
            printf("Printable: %d ('%c')\r\n", c, c);
        }

        if (c == 'q')
            break;
    }

    return 0;
}
