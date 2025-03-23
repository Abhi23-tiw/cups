/*
 * AFL++ fuzzing harness for ippReadIO.
 *
 * Copyright © 2024 by OpenPrinting.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

 #include <cups/file.h>
 #include <cups/string-private.h>
 #include <cups/ipp-private.h>
 
 void load_ipp(cups_file_t *fp);
 
 int main(int argc, char *argv[]) {
     cups_file_t *fp;
 
     if (argc == 1) {
         fp = cupsFileStdin();  // Correct function for stdin
     } else {
         fp = cupsFileOpen(argv[1], "r");
     }
 
     if (fp == NULL) {
         return 1;
     }
 
     load_ipp(fp);
     cupsFileClose(fp);
     return 0;
 }
 
 void load_ipp(cups_file_t *fp) {
     ipp_t *request = ippNew();
     ippReadIO(fp, (ipp_iocb_t)cupsFileRead, 1, NULL, request);
     ippDelete(request);
 }