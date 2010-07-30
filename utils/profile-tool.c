#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* This enum is copied from cm-profile.h */
enum {
    CM_PROFILE_STOP = 0,
    CM_PROFILE_START,
    CM_PROFILE_PREPARE,
    CM_PROFILE_REPORT,
    CM_PROFILE_FLUSH,
    CM_PROFILE_START_TRACE, /* patches the TB to collect backtrace info. */
    CM_PROFILE_REPORT_TRACE,
};

void profile_tool(int command)
{
    /* Use 88 since wanwan borns in 1988 :) */
    __asm__ __volatile__ ( "int $0x88"
                           :
                           : "a" (command)
                           : "cc");
}

void help_info()
{
    printf("usage: profile [COMMAND]\n" \
           "commands:\n" \
           "  -p               prepare the profile\n" \
           "  -s               start the count profile\n" \
           "  -b               start the backtrace profile\n" \
           "  -e               end the profile\n" \
           "  -f               flush the profile information\n" \
           "  -r               get the report of the last profile\n" \
           "  -t               start trace profile\n" \
           "  -o               report trace info\n");
    exit(1);
}

int main(int argc, char *argv[])
{
    int c;

    if(argc != 2)
    {
        printf("Usage: profile #command, -h for help info\n");
        return 1;
    }

    while((c = getopt(argc, argv, "psrfehto")) != -1)
    {
        switch(c) {
        case 'p':
            profile_tool(CM_PROFILE_PREPARE);
            break;
        case 's':
            profile_tool(CM_PROFILE_START);
            break;
        case 'r':
            profile_tool(CM_PROFILE_REPORT);
            break;
        case 'f':
            profile_tool(CM_PROFILE_FLUSH);
            break;
        case 'e':
            profile_tool(CM_PROFILE_STOP);
            break;
        case 't':
            profile_tool(CM_PROFILE_START_TRACE);
            break;
        case 'o':
            profile_tool(CM_PROFILE_REPORT_TRACE);
            break;
        case 'h':
            help_info();
            break;
        default :
            printf("error option : %c\n", c);
            help_info();
        }
    }

    return 1;
}

