#define FILEMAX 10
#define BUFMAX 0x200L

#define MODE	 0
#define HDL	 1
#define BUFLEN	 2 
#define BUFPOS	 3
#define BUFOFS	 4
#define FILPOS	 5
#define DIRTY	 6
#define NAME	 7
#define BUF	 8
#define LAST	 9

class DISK
{
  private :
    long __fhdl[FILEMAX*LAST];
    char __name[FILEMAX*40];

    int fload (long *fp);
    int fflush (long *fp);

  public :
    DISK ();

    int fopen (char *fname, int mode, int type);
    int fread (int fid, ADDRESS addr, int len);
    int fwrite (int fid, ADDRESS addr, int len);
    int fseek (int fid, int pos);
    int fclose (int fid);
    int fdelete (char *fn);
    int frename (char *ofn, char *nfn);
};
