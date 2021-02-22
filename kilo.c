#include <ctype.h>
#include <stdio.h>
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
    // ISIG manages Ctrl-C (SIGINT --> "terminate") and Ctrl-Z (SIGTSTP --> "suspend")
    // IEXTEN for Ctrl-V (paste) and Ctrl-O
    raw_mode_settings.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); // for "local" flags (vs. c_cflag for "control" flags)

    // ICRNL for carriage returns + new lines, fixing Ctrl-M
    // IXON manages Ctrl-S (stops the sending output to terminal) and Ctrl-Q (resumes output to terminal). XON vs XOFF.
    raw_mode_settings.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); // for "input" flags

    // OPOST for disabling \n and \r\n output processing translation, a common default (+ holdover from typwriters/teletype!)
    raw_mode_settings.c_oflag &= ~(OPOST); // "output" flags

    raw_mode_settings.c_cflag |= (CS8);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_mode_settings);
}

int main()
{
    enableRawMode();

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
    {
        if (iscntrl(c)) // control characters are non-printable control characters we shouldn't print to screen (ASCII 0-31 + 127 are all control)
        {
            printf("Control: %d\r\n", c); // need our own \r to return cursors to leftmost side of screen!
        }
        else
        {
            printf("Printable: %d ('%c')\r\n", c, c); // ASCII codes 32-126 are all printable
        }
    }

    return 0;
}
