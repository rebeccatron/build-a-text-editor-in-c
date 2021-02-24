/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/*** definitions ***/

// MACRO to bitwise AND's the character with 00011111
// --> Sets the top 3 bits of the charactor to 0
// --> CTRL key does something similar in terminal: strips bits 5 & 6 from the associated key and sends that as input.
#define CTRL_KEY(k) ((k)&0x1f)

/*** data ***/

struct editorConfig
{
    int screenrows;
    int screencols;
    struct termios users_termios_settings;
};

struct editorConfig E;

/*** terminal ***/

void die(const char *message)
{
    // clear screen & reposition cursor on exit
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(message);
    exit(1);
}

void disableRawMod()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.users_termios_settings) == -1)
        die("tcsetattr while disabling raw mode");
}

void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &E.users_termios_settings) == -1)
        die("tcgetattr");

    atexit(disableRawMod);

    struct termios raw_mode_settings = E.users_termios_settings;

    raw_mode_settings.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);          // for "local" flags
    raw_mode_settings.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); // for "input" flags
    raw_mode_settings.c_oflag &= ~(OPOST);                                  // "output" flags
    raw_mode_settings.c_cflag |= (CS8);                                     // for "control" flags)

    raw_mode_settings.c_cc[VMIN] = 0;
    raw_mode_settings.c_cc[VTIME] = 1; // 1/10 of a second or 100 milliseconds

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_mode_settings) == -1)
        die("tcsetattr while enabling raw mode");
}

// Wait for a single keypress, then return it.
char editorReadKey()
{
    int n_read;
    char c;

    while ((n_read = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (n_read == -1 && errno != EAGAIN)
            die("read");
    }

    return c;
}

int getWindowSize(int *rows, int *cols)
{
    struct winsize ws;

    // TIOCGWINSZ: Terminal IOCtl (which itself stands for Input/Output Control) Get WINdow SiZe.)
    // temporarily walk this if path (because 1 is true) to test
    if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        // C (cursor forward) + B (cursor down) to the 999 max to get to the bottom right of the screen
        // note: C + B specifically  protect against going  "off screen"
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;

        editorReadKey(); // pause a beat to check position before death
        return -1;
    }
    else
    {
        *rows = ws.ws_row;
        *cols = ws.ws_col;
        return 0;
    }
}

/*** output ***/

void editorDrawRows()
{
    int y;

    for (y = 0; y < E.screenrows; y++)
    {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editorRefreshScreen()
{
    // CLEAR THE SCREEN write 4 bytes to terminal:
    // --> 1st byte: \x1b (27 in decimal), the escape character
    // --> 2nd-4th bytes: [2J, the "Erase in Display" command that clears the entire screen
    write(STDOUT_FILENO, "\x1b[2J", 4);

    // RESET CURSOR POSITION
    // --> Moves cursor to position specified by params (line position + column position)
    // --> Using default of (1, 1) or first row at first col. Note: numbers start at 1, not 0!
    write(STDOUT_FILENO, "\x1b[H", 3);

    editorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

// Handles each key press, and soon will map CTRL key combos --> editor functions
void editorProcessKeyPress()
{
    char c = editorReadKey();

    switch (c)
    {
    case CTRL_KEY('q'):

        // clear screen & reposition cursor on quit
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);

        exit(0);
        break;
    }
}

/*** init ***/

void initEditor()
{
    if (getWindowSize(&E.screenrows, &E.screencols) == -1)
        die("getWindowSize");
}

int main()
{
    enableRawMode();
    initEditor();

    while (1)
    {
        // a simple main, KISS.
        editorRefreshScreen();
        editorProcessKeyPress();
    }

    return 0;
}
