// 1D barcodes
// (c) Copyright Adrian Kennard

#include <string.h>
#include <ctype.h>
#include <popt.h>
#include <time.h>
#include <axl.h>
#include <err.h>
#include <stdio.h>
#include <malloc.h>
#include "1dbar.h"

// https://www.gs1.org/docs/barcodes/GS1_General_Specifications.pdf

void
barcode39_opts (barcode_t o)
{
   if (!o.thin)
      o.thin = 2;
   if (!o.thick)
      o.thick = o.thin * 5 / 2;
   int flag = 0;
   void bar (int width, int flags)
   {
      if (!o.barchar)
         flags &= ~(BAR_BELOW + BAR_ABOVE);
      if (flags & BAR_QUIET)
         flag &= ~BAR_BLACK;
      if (o.baradd)
         o.baradd (o.data, width, flags | flag);
      flag ^= BAR_BLACK;
   }
   const char *asciimap[128] = {
      "%U", "$A", "$B", "$C", "$D", "$E", "$F", "$G", "$H", "$I", "$J", "$K", "$L", "$M", "$N", "$O", "$P", "$Q", "$R", "$S", "$T", "$U", "$V", "$W", "$X", "$Y", "$Z", "%A", "%B", "%C", "%D", "$E",   //
      " ", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L", "-", ".", "/O", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "/Z", "$F", "%G", "%H", "%I", "%J",        //
      "%V", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "%K", "%L", "%M", "%N", "%O",     //
      "%W", "+A", "+B", "+C", "+D", "+E", "+F", "+G", "+H", "+I", "+J", "+K", "+L", "+M", "+N", "+O", "+P", "+Q", "+R", "+S", "+T", "+U", "+V", "+W", "+X", "+Y", "+Z", "%P", "%Q", "%R", "%S", "%T",   //
   };
   const char *code39 = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ-. *";
   int count = 0;
   void add39 (int c)
   {
      if (count++)
         bar (o.thin, BAR_GUARD);
      if (o.barchar)
      {
         char t[] = { c };
         o.barchar (o.data, t, 1, 0, 3 * o.thick + 6 * o.thin, 9, BAR_BELOW);
      }
      const char *found = strchr (code39, c);
      if (found)
      {                         // normal
         int S = found - code39;
         int B = (S % 10) + 1;
         S /= 10;
         if (B >= 7)            // ITF style
            B++;
         if (B >= 11)
            B++;
         bar ((B & 1) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((S == 3) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((B & 2) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((S == 0) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((B & 4) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((S == 1) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((B & 8) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((S == 2) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((B == 1 || B == 2 || B == 4 || B == 8) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
      } else
      {                         // special
         bar (o.thin, BAR_DATA | BAR_BELOW);
         bar (c == '%' ? o.thin : o.thick, BAR_DATA | BAR_BELOW);
         bar (o.thin, BAR_DATA | BAR_BELOW);
         bar (c == '+' ? o.thin : o.thick, BAR_DATA | BAR_BELOW);
         bar (o.thin, BAR_DATA | BAR_BELOW);
         bar (c == '/' ? o.thin : o.thick, BAR_DATA | BAR_BELOW);
         bar (o.thin, BAR_DATA | BAR_BELOW);
         bar (c == '$' ? o.thin : o.thick, BAR_DATA | BAR_BELOW);
         bar (o.thin, BAR_DATA | BAR_BELOW);
      }
   }
   void add (unsigned int c)
   {
      if (c >= sizeof (asciimap))
         return;
      const char *map = asciimap[(int) c];
      for (; *map; map++)
         add39 (*map);
   }
   bar (o.thin * 10, BAR_QUIET);
   add39 ('*');
   const char *p = o.value + strlen (o.value) - 1;
   if (*o.value == '*' && *p == '*')    // Assume *'s are start/end markers
      for (o.value++; o.value < p; o.value++)
         add (*o.value);
   else                         // Assume no start end markers
      for (; *o.value; o.value++)
         add (*o.value);
   add39 ('*');
   bar (o.thin * 10, BAR_QUIET);
}

void
barcodeitf_opts (barcode_t o)
{
   if (!o.thin)
      o.thin = 2;
   if (!o.thick)
      o.thick = o.thin * 5 / 2;
   int flag = 0;
   void bar (int width, int flags)
   {
      if (!o.barchar)
         flags &= ~(BAR_BELOW + BAR_ABOVE);
      if (flags & BAR_QUIET)
         flag &= ~BAR_BLACK;
      if (o.baradd)
         o.baradd (o.data, width, flags | flag);
      flag ^= BAR_BLACK;
   }
   bar (o.thin * 10, BAR_QUIET);
   bar (o.thin, BAR_GUARD);
   bar (o.thin, BAR_GUARD);
   bar (o.thin, BAR_GUARD);
   bar (o.thin, BAR_GUARD);
   char prev = 0;
   int sum = 0;
   void digit (char c)
   {
      if (!prev)
      {
         prev = c;
         sum += (c - '0') * 2;
      } else
      {
         if (o.barchar)
         {
            char txt[] = { prev, c };
            o.barchar (o.data, txt, 2, 0, o.thin * 6 + o.thick * 4, 10, BAR_BELOW);
         }
         sum += (c - '0');
         int B = prev - '0';
         int S = c - '0';
         prev = 0;
         if (!B)
            B = 10;
         if (B >= 7)
            B++;
         if (B >= 11)
            B++;
         if (B == 1 || B == 2 || B == 4 || B == 8)
            B += 16;
         if (!S)
            S = 10;
         if (S >= 7)
            S++;
         if (S >= 11)
            S++;
         if (S == 1 || S == 2 || S == 4 || S == 8)
            S += 16;
         bar ((B & 1) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((S & 1) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((B & 2) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((S & 2) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((B & 4) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((S & 4) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((B & 8) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((S & 8) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((B & 16) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
         bar ((S & 16) ? o.thick : o.thin, BAR_DATA | BAR_BELOW);
      }
   }
   for (; *o.value; o.value++)
      if (isdigit (*o.value))
         digit (*o.value);
   if (prev)
      digit ('0' + sum * 9 % 10);       // LUHN check digit
   bar (o.thick, BAR_GUARD);
   bar (o.thin, BAR_GUARD);
   bar (o.thin, BAR_GUARD);
   bar (o.thin * 10, BAR_QUIET);
}

void
barcode128_opts (barcode_t o)
{
   int flag = 0;
   void bar (int width, int flags)
   {
      if (!o.barchar)
         flags &= ~(BAR_BELOW + BAR_ABOVE);
      if (flags & BAR_QUIET)
         flag &= ~BAR_BLACK;
      if (o.baradd)
         o.baradd (o.data, width, flags | flag);
      flag ^= BAR_BLACK;
   }
   const char *map[107] = { "212222", "222122", "222221", "121223", "121322", "131222", "122213",
      "122312", "132212", "221213", "221312", "231212", "112232", "122132",
      "122231", "113222",
      "123122", "123221", "223211", "221132", "221231", "213212", "223112",
      "312131", "311222",
      "321122", "321221", "312212", "322112",
      "322211", "212123", "212321", "232121", "111323", "131123", "131321",
      "112313", "132113", "132311", "211313", "231113", "231311", "112133",
      "112331", "132131", "113123", "113321", "133121", "313121", "211331",
      "231131", "213113", "213311", "213131", "311123", "311321", "331121",
      "312113",
      "312311", "332111",
      "314111", "221411", "431111", "111224", "111422", "121124", "121421",
      "141122", "141221", "112214", "112412", "122114", "122411", "142112",
      "142211", "241211", "221114", "413111", "241112", "134111", "111242",
      "121142", "121241", "114212", "124112", "124211", "411212", "421112",
      "421211",
      "212141", "214121",
      "412121", "111143", "111341", "131141", "114113", "114311", "411113",
      "411311", "113141", "114131", "311141", "411131", "211412", "211214",
      "211232", "2331112"
   };
   int c = 0,
      n = 0;
   void addcode (int v, int flags)
   {
      c += (n ? n : 1) * v;
      n++;
      if (v < 0 || v >= (int) (sizeof (map) / sizeof (*map)))
         errx (1, "Bad code 128: %d", v);
      const char *p = map[v];
      while (*p)
         bar (*p++ - '0', flags);
   }
   bar (10, BAR_QUIET);
   char set = 0;
   for (; *o.value; o.value++)
   {
      char newset = 0;
      // TODO we can be way smarter and use shift characters and so on...
      if (isdigit (*o.value))
      {                         // Maybe use set C
         int r = 1;
         while (isdigit (o.value[r]))
            r++;
         if (r >= 4)            //(r > 4 && !(r & 1)) || (r == 4 && !o.value[r]))
            newset = 'C';
      }
      if (*o.value < 0x02)
         newset = 'A';
      if (*o.value >= 0x60)
         newset = 'B';
      if (!newset && set == 'C' && (!isdigit (*o.value) || !isdigit (o.value[1])))
         newset = 'B';
      if (!newset && !set)
         newset = 'B';
      if (!set)
         addcode (103 + ((set = newset) - 'A'), BAR_GUARD);
      else if (newset && set != newset)
         addcode (101 - ((set = newset) - 'A'), BAR_DATA);
      if (set == 'A')
      {
         if (o.barchar)
            o.barchar (o.data, o.value, 1, 0, 11, 7, BAR_BELOW);
         addcode (*o.value < 0x20 ? *o.value + 0x40 : *o.value - 0x20, BAR_DATA | BAR_BELOW);
      } else if (set == 'B')
      {
         if (o.barchar)
            o.barchar (o.data, o.value, 1, 0, 11, 7, BAR_BELOW);
         addcode (*o.value - 0x20, BAR_DATA | BAR_BELOW);
      } else if (set == 'C')
      {
         if (o.barchar)
            o.barchar (o.data, o.value, 2, 0, 11, 5, BAR_BELOW);
         addcode ((*o.value - '0') * 10 + o.value[1] - '0', BAR_DATA | BAR_BELOW);
         o.value++;
      }
   }
   addcode (c % 103, BAR_DATA);
   addcode (106, BAR_GUARD);
   bar (10, BAR_QUIET);
}

void
barcodeean_opts (barcode_t o)
{                               // This handles a range of EAN/UPC formats including UPC-E (7 digits), UPC-A (12 digits), EAN-8 (8 digits), EAN=13 (13 digits)
   int flag = 0;
   void bar (int width, int flags)
   {
      if (!o.barchar)
         flags &= ~(BAR_BELOW + BAR_ABOVE);
      if (flags & BAR_QUIET)
         flag &= ~BAR_BLACK;
      if (o.baradd)
         o.baradd (o.data, width, flags | flag);
      flag ^= BAR_BLACK;
   }
   void chars (const char *txt, int n, int dx, int w, int cw, int flags)
   {
      if (o.barchar)
         o.barchar (o.data, txt, n, dx, w, cw, flags);
   }
   const char *p,
    *a = NULL;
   for (p = o.value; *p && isdigit (*p); p++);
   int l = p - o.value;
   if (*p == '-' && isdigit (p[1]))
   {                            // Add-on
      p++;
      a = p;
      while (*p && isdigit (*p))
         p++;
   }
   if (*p)
      return;                   // not digits
   if (o.checkdigit)
   {
      int c = 0;
      for (int n = 0; n < l - 1; n++)
         c += (o.value[n] - '0') * (((l - n) & 1) ? 1 : 3);
      o.value[l - 1] = '0' + (10 - (c % 10)) % 10;
   }
   // Check digit

   int reverse = 0,
      q;
   void digit (int d, int flags)
   {
      const char *ean[] = { "3211", "2221", "2122", "1411", "1132", "1231", "1114", "1312", "1213",
         "3112"
      };
      const char *s = ean[d];
      if (reverse & 1)
      {                         // reverse
         const char *e = s + strlen (s) - 1;
         while (e >= s)
            bar (*e-- - '0', BAR_DATA | flags);
      } else
         while (*s)
            bar (*s++ - '0', BAR_DATA | flags);
      reverse >>= 1;
   }
   int left = 10,
      right = 10;               // quiet zones
   if (l == 13)
      left = 11, right = 7;     // EAN-13
   if (l == 12)
      left = 9, right = 9;      // UPC-A
   if (l == 8)
      left = 7, right = 7;      // EAN-8
   if (l == 7)
      left = 9, right = 7;      // UPC-E

   void guard (int n)
   {
      while (n--)
         bar (1, BAR_GUARD);    // Start
   }
   if (l == 7)
   {                            // UPC-E
      const int cs[] = { 0x07, 0x0b, 0x13, 0x23, 0x0d, 0x19, 0x31, 0x15, 0x25, 0x29 };
      reverse = cs[o.value[l - 1] - '0'];
      chars ("0", 1, 0, 6, 5, BAR_BELOW | BAR_LEFT);    // Left of quiet (as per GS-1 spec)
      bar (left, BAR_QUIET | BAR_BELOW);
      guard (3);
      chars (o.value, 6, 0, 7 * 6 + 1, 7, BAR_BELOW);
      for (q = 0; q < 6; q++)
         digit (o.value[q] - '0', BAR_BELOW);
      guard (6);
      bar (right, BAR_QUIET | BAR_BELOW);
      chars (o.value + q, 1, 0, 6, 5, BAR_BELOW | BAR_RIGHT);   // Right of quiet (as per GS-1 spec)
   } else if (l == 8)
   {                            // EAN-8
      chars ("<", 1, 0, 8, 7, BAR_BELOW | BAR_LEFT);    // Left of quiet (as per GS-1 spec)
      bar (left, BAR_QUIET | BAR_BELOW);
      guard (3);
      for (q = 0; q < l; q++)
      {
         if (!q || q == 4)
            chars (o.value + q, 4, q ? -1 : 0, 7 * 4 + 1, 7, BAR_BELOW);
         digit (o.value[q] - '0', BAR_BELOW);
         if (q == 3)
            guard (5);
      }
      guard (3);
      bar (right, BAR_QUIET | BAR_BELOW);
      if (!a)
         chars (">", 1, 0, 8, 7, BAR_BELOW | BAR_RIGHT);        // Right of quiet (as per GS-1 spec)
   } else if (l == 12)
   {                            // UPC-A
      chars (o.value, 1, 0, 6, 5, BAR_BELOW | BAR_LEFT);        // Left of quiet (as per GS-1 spec)
      bar (left, BAR_QUIET | BAR_BELOW);
      guard (3);
      digit (o.value[0] - '0', 0);
      for (q = 1; q < l - 1; q++)
      {
         if (q == 1 || q == 6)
            chars (o.value + q, 5, q > 1 ? -1 : 0, 7 * 5 + 1, 7, BAR_BELOW);
         digit (o.value[q] - '0', BAR_BELOW);
         if (q == 5)
            guard (5);
      }
      digit (o.value[q] - '0', 0);
      guard (3);
      bar (right, BAR_QUIET | BAR_BELOW);
      chars (o.value + q, 1, 0, 6, 5, BAR_BELOW | BAR_RIGHT);   // Right of quiet (as per GS-1 spec)
   } else if (l == 13)
   {                            // EAN-13
      const int leftswap[] = { 0, 0x34, 0x2c, 0x1c, 0x32, 0x26, 0x0e, 0x2a, 0x1a, 0x16 };
      reverse = leftswap[o.value[0] - '0'];
      chars (o.value, 1, 0, 8, 7, BAR_BELOW | BAR_LEFT);        // Left of quiet (as per GS-1 spec)
      bar (left, BAR_QUIET | BAR_BELOW);
      guard (3);
      for (q = 1; q < l; q++)
      {
         if (q == 1 || q == 7)
            chars (o.value + q, 6, q > 1 ? -1 : 0, 6 * 7 + 1, 7, BAR_BELOW);
         digit (o.value[q] - '0', BAR_BELOW);
         if (q == 6)
            guard (5);
      }
      guard (3);
      bar (right, BAR_QUIET | BAR_BELOW);
      if (!a)
         chars (">", 1, 0, 8, 7, BAR_BELOW | BAR_RIGHT);        // Right of quiet (as per GS-1 spec)
   }
   if (a)
   {                            // Add-on
      int al = strlen (a);
      if (al == 2)
      {
         int m = (((a[0] - '0') * 10 + (a[1] - '0')) % 4);
         if (m & 1)
            reverse |= 2;
         if (m & 2)
            reverse |= 1;
      } else if (al == 5)
      {
         int m = (((a[0] - '0') + (a[2] - '0') + (a[4] - '0')) * 3 + ((a[1] - '0') + (a[3] - '0')) * 9) % 10;
         int cs[] = { 0x03, 0x05, 0x09, 0x11, 0x06, 0x0c, 0x18, 0x0a, 0x12, 0x14 };
         reverse = cs[m];
      }
      bar (1, BAR_GUARD | BAR_ABOVE);
      bar (1, BAR_GUARD | BAR_ABOVE);
      bar (2, BAR_GUARD | BAR_ABOVE);
      for (q = 0; q < al; q++)
      {
         chars (a + q, 1, 0, 9, 7, BAR_ABOVE | BAR_LEFT);
         digit (a[q] - '0', BAR_ABOVE);
         if (q + 1 < al)
         {
            bar (1, BAR_GUARD | BAR_ABOVE);
            bar (1, BAR_GUARD | BAR_ABOVE);
         }
      }
      //bar (5, BAR_QUIET|BAR_BELOW); // GS-1 spec - silly
      bar (7, BAR_QUIET | BAR_BELOW);
      chars (">", 1, 0, 7, 7, BAR_ABOVE | BAR_RIGHT);
   }
}

void
barcodetelepen_opts (barcode_t o)
{
   if (!o.thin)
      o.thin = 1;
   if (!o.thick)
      o.thick = o.thin * 3;
   if (o.numeric)
   {
      int l = 0;
      for (const char *c = o.value; *c; c++)
         if (isdigit (*c))
            l++;
      char buf[(l + 1) / 2];
      l = 0;
      for (const char *c = o.value; *c; c++)
         if (isdigit (*c))
         {
            if (l & 1)
               buf[l / 2] = 27 + (buf[l / 2] - 17) * 10 + *c - '0';
            else
               buf[l / 2] = 17 + *c - '0';
            l++;
         }
   }
   int flag = 0;
   void bar (int width, int flags)
   {
      if (!o.barchar)
         flags &= ~(BAR_BELOW + BAR_ABOVE);
      if (flags & BAR_QUIET)
         flag &= ~BAR_BLACK;
      if (o.baradd)
         o.baradd (o.data, width, flags | flag);
      flag ^= BAR_BLACK;
   }
   void chars (const char *txt, int n, int dx, int w, int cw, int flags)
   {
      if (o.barchar)
         o.barchar (o.data, txt, n, dx, w, cw, flags);
   }
   // Compose bits
   unsigned char bits[o.len + 3];
   int q = 0;
   void add (unsigned char c)
   {
      c &= 0x7F;
      for (int b = 0; b < 7; b++)
         if (c & (1 << b))
            c ^= 0x80;          // parity
      bits[q++] = c;
   }
   add ('_');
   int csum = 0;
   for (int i = 0; i < o.len; i++)
   {
      add (o.value[i]);
      csum += (o.value[i] & 0x7F);
   }
   csum = 127 - (csum % 127);
   if (csum == 127)
      csum = 0;
   add (csum);
   add ('z');
   // Make barcode
   bar (o.thin * 8, BAR_QUIET); // Assuming 8 is sensible
   int n = q * 8;
   chars (o.value, o.len, 0, n * 2, 7, BAR_BELOW);
   q = 0;
   while (q < n)
   {
      if (bits[q / 8] & (1 << (q & 7)))
      {                         // 1
         bar (o.thin, BAR_DATA | BAR_BELOW);
         if (q + 1 < n)
            bar (o.thin, BAR_DATA | BAR_BELOW);
         q++;
         continue;
      }
      // 0
      q++;
      if (!(bits[q / 8] & (1 << (q & 7))))
      {                         // 00
         bar (o.thick, BAR_DATA | BAR_BELOW);
         bar (o.thin, BAR_DATA | BAR_BELOW);
         q++;
         continue;
      }
      q++;
      // 01
      if (!(bits[q / 8] & (1 << (q & 7))))
      {                         // 010
         bar (o.thick, BAR_DATA | BAR_BELOW);
         bar (o.thick, BAR_DATA | BAR_BELOW);
         q++;
         continue;
      }
      q++;
      // 011
      bar (o.thin, BAR_DATA | BAR_BELOW);       // Start run
      bar (o.thick, BAR_DATA | BAR_BELOW);
      while (q < n && bits[q / 8] & (1 << (q & 7)))
      {                         // 1
         bar (o.thin, BAR_DATA | BAR_BELOW);
         bar (o.thin, BAR_DATA | BAR_BELOW);
         q++;
      }
      // 10
      q++;
      bar (o.thin, BAR_DATA | BAR_BELOW);       // End run
      bar (o.thick, BAR_DATA | BAR_BELOW);
   }
   bar (o.thin * 8, BAR_QUIET); // Assuming 8 is sensible
}

#ifndef LIB

int debug = 0;

int
main (int argc, const char *argv[])
{
   int format = 0;
   double h = -1;               // height
   double whiteborder = 0;      // Extra white border
   double fh = -1;              // font height
   double fontadjust = 0;       // font size increase
   int bleed = 0;               // Edge white for over printing black bleed
   int border = 4;              // GTIN-4 border
   int left = -1;               // Left extra border
   int right = -1;              // Right extra border
   int invert = 0;              // Invert
   double unitsize = -1;        // mm size of units
   double unitdpi = -1;         // dpi size to set mm size of units
   const char *font = "OCRB,OCR-B,sans-serif";
   int kicad = 0;
   const char *kicadfont = "OCR-B";
   const char *kicadlayer = "F.Cu";
   char *code = NULL;
   poptContext optCon;          // context for parsing command-line options
   {                            // POPT
      const struct poptOption optionsTable[] = {
         {"gtin", 'g', POPT_ARG_VAL, &format, 'g', "GTIN"},
         {"ean", 0, POPT_ARG_VAL | POPT_ARGFLAG_DOC_HIDDEN, &format, 'g', "EAN"},
         {"upc", 0, POPT_ARG_VAL | POPT_ARGFLAG_DOC_HIDDEN, &format, 'g', "UPC"},
         {"itf", 0, POPT_ARG_VAL, &format, 'i', "ITF"},
         {"c39", 0, POPT_ARG_VAL, &format, '3', "Code 39"},
         {"c128", 0, POPT_ARG_VAL, &format, 'c', "Codebar 128"},
         {"telepen", 0, POPT_ARG_VAL, &format, 't', "Telepen"},
         {"telepen-numeric", 0, POPT_ARG_VAL, &format, 'n', "Telepen"},
         {"mm", 's', POPT_ARG_DOUBLE, &unitsize, 0, "Unit size", "mm"},
         {"dpi", 'd', POPT_ARG_DOUBLE, &unitdpi, 0, "Unit dpi", "dpi"},
         {"unit-size", 0, POPT_ARGFLAG_DOC_HIDDEN | POPT_ARG_DOUBLE, &unitsize, 0, "Unit size", "mm"},
         {"unit-dpi", 0, POPT_ARGFLAG_DOC_HIDDEN | POPT_ARG_DOUBLE, &unitdpi, 0, "Unit dpi", "dpi"},
         {"height", 'h', POPT_ARG_DOUBLE, &h, 0, "Height", "mm"},
         {"white-border", 0, POPT_ARG_DOUBLE, &whiteborder, 0, "White border", "mm"},
         {"font-height", 'H', POPT_ARG_DOUBLE, &fh, 0, "Font height", "mm"},
         {"font", 'f', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &font, 0, "Font", "font"},
         {"left", 0, POPT_ARG_INT, &left, 0, "Left quiet zone", "units"},
         {"right", 0, POPT_ARG_INT, &right, 0, "Right quiet zone", "units"},
         {"bleed", 'b', POPT_ARG_INT, &bleed, 0, "Allow print bleed", "% of bar"},
         {"code", 'c', POPT_ARG_STRING, &font, 0, "Code", "code"},
         {"invert", 0, POPT_ARG_NONE, &invert, 0, "Invert"},
         {"kicad", 0, POPT_ARG_NONE, &kicad, 0, "KiCad module"},
         {"kicad-font", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &kicadfont, 0, "KiCad font", "font"},
         {"kicad-layer", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &kicadlayer, 0, "KiCad layer", "layer"},
         {"debug", 'v', POPT_ARG_NONE, &debug, 0, "Debug"},
         POPT_AUTOHELP {}
      };

      optCon = poptGetContext (NULL, argc, argv, optionsTable, 0);
      poptSetOtherOptionHelp (optCon, "[code]");

      int c;
      if ((c = poptGetNextOpt (optCon)) < -1)
         errx (1, "%s: %s\n", poptBadOption (optCon, POPT_BADOPTION_NOALIAS), poptStrerror (c));

      if (poptPeekArg (optCon) && !code)
         code = (char *) poptGetArg (optCon);

      if (poptPeekArg (optCon) || !format || !code)
      {
         poptPrintUsage (optCon, stderr, 0);
         return -1;
      }
   }
   if (unitdpi >= 0 && unitsize >= 0)
      errx (1, "--dpi or --mm");
   if (unitdpi > 0)
      unitsize = 25.4 / unitdpi;
   if (!fontadjust)
      fontadjust = kicad ? 0.75 : 1.2;
   if (kicad)
      invert ^= 1;

   int len = strlen (code);
   // Some defaults
   if (format == 'g')
   {                            // GTIN / EAN / UPC
      for (len = 0; code[len] && isdigit (code[len]); len++);   // Allow for add-ons
      if (len == 14)
      {                         // GTIN-14 (ITF)
         if (left < 0)
            left = border / 2;
         if (right < 0)
            right = border / 2;
         if (unitsize < 0)
            unitsize = 0.635;   // 25 mil
         if (fh < 0)
            fh = 5;
         if (h < 0)
            h = 31.75 + border + fh / unitsize; // 1.25 (height of bars)
      } else
      {                         // Normal EAN/UPC/GTIN
         if (h < 0)
         {                      // Default height
            if (len == 13)
               h = 25.93;
            else if (len == 12)
               h = 25.93;
            else if (len == 8)
               h = 21.31;
            else if (len == 7)
               h = 25.93;
         }
      }
   }
   if (unitsize < 0)
      unitsize = 0.33;
   if (fh < 0)
      fh = unitsize * 28 / 3;
   if (h < 0)
      h = 20;
   if (left < 0)
      left = 0;
   if (right < 0)
      right = 0;
   // Check digit
   if (format == 'g')
   {
      int m = ((len & 1) ? 1 : 3),
         p,
         t = 0;
      for (p = 0; p < len - 1; p++)
      {
         t += m * (code[p] - '0');
         m = 4 - m;
      }
      t %= 10;
      t = 10 - t;
      t %= 10;
      code[p] = '0' + t;
   }

   if (format == 'g' && len == 12 && code[0] == '0')
   {                            // Consider reducing to UPC-E
      if (code[10] >= '5' && code[10] <= '9' && code[6] == '0' && code[7] == '0' && code[8] == '0' && code[9] == '0' && code[5] != '0')
         len = asprintf (&code, "%.5s%c%c", code + 1, code[10], code[11]);
      else if (code[5] == '0' && code[6] == '0' && code[7] == '0' && code[8] == '0' && code[9] == '0' && code[4] != '0')
         len = asprintf (&code, "%.4s%c4%c", code + 1, code[10], code[11]);
      else if (code[3] >= '0' && code[3] <= '2' && code[4] == '0' && code[5] == '0' && code[6] == '0' && code[7] == '0')
         len = asprintf (&code, "%.2s%.3s%c%c", code + 1, code + 8, code[3], code[11]);
      else if (code[3] >= '3' && code[3] <= '9' && code[4] == '0' && code[5] == '0' && code[6] == '0' && code[7] == '0' && code[8] == '0')
         len = asprintf (&code, "%.3s%.2s3%c", code + 1, code + 9, code[11]);
   }

   int w = 0;
   int u = 1;
   int q = 0;
   if ((format == 'g' && len == 14) || format == 'i' || format == '3')
      u = 2;
   xml_t root = xml_tree_new ("svg");
   xml_t rect = xml_element_add (root, "rect");
   xml_t text = NULL;
   if (fh > 0)
   {
      text = xml_element_add (root, "g");
      xml_add (text, "@font-family", font);
      xml_addf (text, "@font-size", "%.2f", fontadjust * fh * u / unitsize);
   }
   char *d;
   size_t dlen;
   FILE *path = open_memstream (&d, &dlen);
   void baradd (void *ptr, int n, int flags)
   {
      if (invert ? (!(flags & BAR_BLACK)) : (flags & BAR_BLACK))
      {
         q++;
         double l = h * u,
            t = 0;
         if (fh > 0)
         {
            if (flags & BAR_ABOVE)
            {
#if 0
               // Note, the GS1 spec has add on 21.9mm high for UPC-A and EAN-13 which is clearly a mistake, and also allows far to little space for digits

               t += fh * u * 0.844;     // space for digits above
               if (len == 12)
                  l -= fh * u * 1.844;
               else
                  l -= fh * u * 1.308;
#else
               // This is more logical, allows space for digits
               t += fh * u * 1; // Allow space
               if (len == 12)
                  l -= fh * u * 2;
               else
                  l -= fh * u * 1.464;
#endif
            } else if ((flags & BAR_BELOW) || len == 14)
               l -= fh * u;     // space for digits below
            else
               l -= fh * u * 0.464;     // shorted bars anyway when digits apply
         }
         if (kicad)
            fprintf (path, "(fp_rect (start %.2f %.2f) (end %.2f %.2f)(stroke (width 0) (type solid)) (fill solid) (layer %s))\n", (double) w * unitsize / u, (double) t / u, (double) (w + n) * unitsize / u, (double) (t + l) / u, kicadlayer);
         else
            fprintf (path, "M%d %.2fh%dv%.2fh%dz", w, t / unitsize, n, l / unitsize, -n);
      }
      w += n;
   }
   void barchar (void *ptr, const char *txt, int c, int dx, int n, int cw, int flags)
   {
      if (!text)
         return;
      if (kicad)
      {
         double X = (flags & (BAR_RIGHT | BAR_LEFT)) ? (w + dx) : (double) (w + dx) + (double) n / 2;
         double Y = (flags & BAR_ABOVE) ? fh * u * cw / unitsize * 0.92 / 7 : h * u / unitsize - 0.1;
         double S = (cw < 7) ? fontadjust * fh * cw / 7 : fontadjust * fh;
         // Adjust to kicad units
         X = X * unitsize / u;
         Y = Y * unitsize / u;
         fprintf (path, "(fp_text user \"%.*s\" (at %.2f %.2f) (layer %s) (effects (font ", c, txt, X, Y, kicadlayer);
         if (*kicadfont)
            fprintf (path, "(face \"%s\") ", kicadfont);
         fprintf (path, "(size %.2f %.2f) (thickness 0.1)) (justify ", S, S);
         if (flags & BAR_LEFT)
            fprintf (path, "left ");
         else if (flags & BAR_RIGHT)
            fprintf (path, "right ");
         fprintf (path, "bottom)))\n");
         return;
      }
      xml_t t = xml_addf (text, "+text", "%.*s", c, txt);
      if (cw < 7)
         xml_addf (t, "@font-size", "%.2f", fontadjust * fh * u * cw / unitsize / 7);
      if (flags & (BAR_RIGHT | BAR_LEFT))
         xml_addf (t, "@x", "%d", (w + dx));
      else
         xml_addf (t, "@x", "%.1f", (double) (w + dx) + (double) n / 2);
      if (flags & BAR_ABOVE)
         xml_addf (t, "@y", "%.2f", fh * u * cw / unitsize * 0.92 / 7);
      else
         xml_addf (t, "@y", "%.2f", h * u / unitsize - 0.1);
      if (!(flags & BAR_LEFT))
         xml_addf (t, "@text-anchor", (flags & BAR_RIGHT) ? "end" : "middle");
   }
   if (kicad)
   {
      fprintf (path, "(module Barcode (layer %s)\n", kicadlayer);
      fprintf (path, "(attr exclude_from_pos_files exclude_from_bom allow_soldermask_bridges)\n");
      fprintf (path, "(fp_text reference REF** (at 0 -3) (layer F.SilkS) hide (effects (font (size 1 1) (thickness 0.15)) (justify left)))\n");
      fprintf (path, "(fp_text value %s (at 0 -1) (layer F.Fab) hide (effects (font (size 1 1) (thickness 0.15)) (justify left)))\n", code);
   }
   if (format == 'g')
   {
      if (len == 14)
         u = 2;                 // Double units to allow 2.5:1 ITF
      w += left * u;
      if (len == 14)
       barcodeitf (code, baradd: &baradd, thin:u);
      // GTIN-14 packaging label
      else
       barcodeean (code, baradd: &baradd, barchar:&barchar);
      // EAN/UPC/GTIN product label
      w += right * u;
   }
   if (format == 'i')
    barcodeitf (code, baradd: &baradd, barchar:&barchar);
   if (format == '3')
    barcode39 (code, baradd: &baradd, barchar:&barchar);
   if (format == 'c')
    barcode128 (code, baradd: &baradd, barchar:&barchar);
   if (format == 't')
    barcodetelepen (code, baradd: &baradd, barchar: &barchar, len:strlen (code));
   if (format == 'n')
    barcodetelepen (code, baradd: &baradd, barchar: &barchar, numeric:1);
   if (kicad)
   {
      if (strstr (kicadlayer, "Cu"))
      {
         double l = (double) -border,
         r = (double) w *unitsize / u + border,
           t = (double) h + border,
            b = (double) -border;
         fprintf (path, "(fp_line (start %.2f %.2f) (end %.2f %.2f) (layer F.CrtYd) (width 0.1))\n", l, b, l, t);
         fprintf (path, "(fp_line (start %.2f %.2f) (end %.2f %.2f) (layer F.CrtYd) (width 0.1))\n", l, t, r, t);
         fprintf (path, "(fp_line (start %.2f %.2f) (end %.2f %.2f) (layer F.CrtYd) (width 0.1))\n", r, t, r, b);
         fprintf (path, "(fp_line (start %.2f %.2f) (end %.2f %.2f) (layer F.CrtYd) (width 0.1))\n", r, b, l, b);
         fprintf (path, "(fp_poly (pts (xy %f %f) (xy %f %f) (xy %f %f) (xy %f %f)) (layer F.Mask) (width 0))\n", l, b, l, t, r, t, r, b);
         fprintf (path, "(zone (net 0) (net_name \"\") (layer \"F.Cu\") (hatch edge 0.508)\n" "(connect_pads (clearance 0))\n"
                  "(min_thickness 0.254)\n" "(keepout (tracks not_allowed) (vias not_allowed) (copperpour not_allowed) (footprints not_allowed))\n" "(fill (thermal_gap 0.508) (thermal_bridge_width 0.508))\n" "(polygon (pts (xy %f %f) (xy %f %f) (xy %f %f) (xy %f %f))))\n", l, b, l, t, r, t, r, b);
      }
      fprintf (path, ")\n");
   }
   fclose (path);
   if (len == 14)
   {                            // GTIN-14
      xml_t t = xml_addf (text, "+text", "%.1s %.2s %.5s %.5s %.1s", code + 0, code + 1, code + 3, code + 8, code + 13);
      xml_addf (t, "@textLength", "%d", 14 * (5 + 5 + 2 + 2 + 2));
      xml_addf (t, "@x", "%d", border * u + 10 * u + 4 * u);
      xml_addf (t, "@y", "%.2f", h * u / unitsize);
   }
   xml_element_set_namespace (root, xml_namespace (root, NULL, "http://www.w3.org/2000/svg"));
   xml_namespace (root, "^xlink", "http://www.w3.org/1999/xlink");
   root->tree->encoding = NULL;
   xml_t p = xml_element_add (root, "path");
   xml_add (p, "@d", d);
   xml_add (p, "@fill", "black");
   if (bleed)
   {
      xml_addf (p, "@stroke-width", "%.2f", (double) bleed * u / 100);
      xml_add (p, "@stroke", "white");
   } else
      xml_add (p, "@stroke", "none");
   if (unitsize >= 0)
   {
      xml_addf (root, "@viewBox", "0 0 %d %.2f", w, h * u / unitsize);
      xml_addf (root, "@width", "%.2fmm", unitsize * w / u);
      xml_addf (root, "@height", "%.2fmm", h);
   } else
   {
      xml_addf (root, "@width", "%d", w);
      xml_addf (root, "@height", "%d", h * u / unitsize);
   }
   xml_addf (rect, "@width", "%d", w);
   xml_addf (rect, "@height", "%.2f", h * u / unitsize);
   xml_add (rect, "@fill", "white");
   if (whiteborder)
   {
      xml_add (rect, "@stroke", "white");
      xml_addf (rect, "@stroke-width", "%.2f", whiteborder * 2 * u / unitsize);
   } else
      xml_add (rect, "@stroke", "none");
   if (format == 'g' && len == 14)
   {
      rect = xml_element_add (root, "rect");
      xml_addf (rect, "@x", "%d", border * u / 2);
      xml_addf (rect, "@y", "%d", border * u / 2);
      xml_addf (rect, "@width", "%d", w - border * u);
      xml_addf (rect, "@height", "%.2f", h / unitsize * u - border * u - fh * u / unitsize);
      xml_add (rect, "@fill", "none");
      xml_add (rect, "@stroke", "black");
      xml_addf (rect, "@stroke-width", "%d", border * u);
   }
   if (kicad)
      printf ("%s", d);         // Kicad is all in path
   else
      xml_write (stdout, root);
   free (d);
   xml_tree_delete (root);
   poptFreeContext (optCon);
   return 0;
}
#endif
