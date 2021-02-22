#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

// config object
struct termios users_termios_settings;

void die(const char *message)
{
    // most c library functions will set the global errno variable to indicate the error code.
    // perror looks at the global errnoo & prints a descriptive error for it
    // perror also formats any message you wish to provide to make debugging easier
    perror(message);
    exit(1);
}

void disableRawMod()
{
    // TCSCAFLUSH --> "drain output, flush input"
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &users_termios_settings) == -1)
        die("tcsetattr while disabling raw mode");
}

// configures terminal to provide a more interactive experience in main
void enableRawMode()
{

    // nab the user's default terminal settings
    if (tcgetattr(STDIN_FILENO, &users_termios_settings) == -1)
        die("tcgetattr");

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

    // VMIN + VTIME are indices into the c_cc field (stands for "control characters") --> byte array with terminal settings
    // VMIN: Min number of bytes of input needed before read() can return. Setting it to 0 makes it respond as soon as ANY input is available to read.
    // VTIME: Max amount of time to wait between read() returns, in tenths of a second
    raw_mode_settings.c_cc[VMIN] = 0;
    raw_mode_settings.c_cc[VTIME] = 1; // 1/10 of a second or 100 milliseconds

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_mode_settings) == -1)
        die("tcsetattr while enabling raw mode");
}

int main()
{
    enableRawMode();

    while (1)
    {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");

        if (iscntrl(c)) // control characters are non-printable control characters we shouldn't print to screen (ASCII 0-31 + 127 are all control)
        {
            printf("Control: %d\r\n", c); // need our own \r to return cursors to leftmost side of screen!
        }
        else
        {
            printf("Printable: %d ('%c')\r\n", c, c); // ASCII codes 32-126 are all printable
        }

        if (c == 'q')
            break;
    }

    return 0;
}
