/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <ctype.h>
#include "bsp.h"
#include "strfunc.h"

LOCALFUNC XMEDIA_RET atoul(IN CHAR *str,OUT unsigned long * pulValue);
LOCALFUNC XMEDIA_RET atoulx(IN CHAR *str,OUT unsigned long * pulValue);

/*****************************************************************************
 Prototype    : StrToNumber
 Calls        : isdigit

 Called  By   :

 History        :
   Modification : Created function
*****************************************************************************/

XMEDIA_RET StrToNumber(IN CHAR *str , OUT unsigned long * pulValue)
{
    if ( *str == '0' && (*(str+1) == 'x' || *(str+1) == 'X') )
    {
        if (*(str+2) == '\0')
        {
            return XMEDIA_FAILURE;
        }
        else
        {
            return atoulx(str+2,pulValue);
        }
    }
    else
    {
        return atoul(str,pulValue);
    }
}

/*****************************************************************************
 Prototype    : atoul
 Calls        : isdigit

 Called  By   :

 History        :
   Modification : Created function
*****************************************************************************/
XMEDIA_RET atoul(IN CHAR *str,OUT unsigned long * pulValue)
{
    unsigned long ulResult=0;

    while (*str)
    {
        if (isdigit((int)*str))
        {
            if ((ulResult<429496729) || ((ulResult==429496729) && (*str<'6')))
            {
                ulResult = ulResult*10 + (*str)-48;
            }
            else
            {
                *pulValue = ulResult;
                return XMEDIA_FAILURE;
            }
        }
        else
        {
            *pulValue=ulResult;
            return XMEDIA_FAILURE;
        }
        str++;
    }
    *pulValue=ulResult;
    return XMEDIA_SUCCESS;
}

/*****************************************************************************
 Prototype    : atoulx
 Calls        : toupper
                isdigit

 Called  By   :

 History        :
   Modification : Created function
*****************************************************************************/
#define ASC2NUM(ch) (ch - '0')
#define HEXASC2NUM(ch) (ch - 'A' + 10)

XMEDIA_RET  atoulx(IN CHAR *str,OUT unsigned long * pulValue)
{
    unsigned long  ulResult=0;
    UCHAR ch;

    while (*str)
    {
        ch=toupper(*str);
        if (isdigit(ch) || ((ch >= 'A') && (ch <= 'F' )))
        {
            if (ulResult < 0x200000000)
            {
                ulResult = (ulResult << 4) + ((ch<='9')?(ASC2NUM(ch)):(HEXASC2NUM(ch)));
            }
            else
            {
                *pulValue=ulResult;
                return XMEDIA_FAILURE;
            }
        }
        else
        {
            *pulValue=ulResult;
            return XMEDIA_FAILURE;
        }
        str++;
    }

    *pulValue=ulResult;
    return XMEDIA_SUCCESS;
}