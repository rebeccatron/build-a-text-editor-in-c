#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

// config object
struct termios users_termios_settings;

void disableRawMod()
{
    // TCSCAFLUSH --> "drain output, flush input"
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &users_termios_settings);
}

// configures terminal to provide a more interactive experience in main
void enableRawMode()
{

    // nab the user's default terminal settings
    tcgetattr(STDIN_FILENO, &users_termios_settings);

    // then, restore those settings "at exit"
    // man: atexit -- register a function to be called on exit
    atexit(disableRawMod);

    // make a copy, so we can configure raw mode
    struct termios raw_mode_settings = users_termios_settings;

    // ECHO causes each key to print to terminal --> not needed for user interface in raw mode
    // ICANON manages canonical mode. With it off, we'll read input byte-by-byte not line-by-line
    raw_mode_settings.c_lflag &= ~(ECHO | ICANON); // for "local" flags (vs. c_cflag for "control" flags)
    // ^ turns off ONLY the echo + icanon bits via bitwise NOT "anded" with the existing fields flag so that the ECHO bit turns "off".

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_mode_settings);
}

int main()
{
    char c;

    enableRawMode();

    // read 1 byte from standard input into char variable c
    // stop once read fails to return 1 byte read (i.e. nothing left to read --> EOF)
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
        ;

    return 0;
}
