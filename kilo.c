/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** definitions ***/

// MACRO to bitwise AND's the character with 00011111
// --> Sets the top 3 bits of the charactor to 0
// --> CTRL key does something similar in terminal: strips bits 5 & 6 from the associated key and sends that as input.
#define CTRL_KEY(k) ((k)&0x1f)

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

        if (c == CTRL_KEY('q')) // frees up the actual 'q' character!
            break;
    }

    return 0;
}
