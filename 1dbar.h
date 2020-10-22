// 1D barcode generators
// (c) 2019 Adrian Kennard

// Callback functions (return width added)
typedef int baradd_t (void *data, int w, int flags);    // function that adds next bar
typedef void barchar_t (void *data, const char *text, int n, int dx, int w, int h,int flags); // function that adds text (n chars of text, at offset dx w units, font based on h width

// Barcode generation functions - these call the callback functions
// baradd is called in turn for each bar from left to right starting with quiet zone
// barchar is called for characters or strings, interleaved with the bars
// return value is number of units total (including quiet zones). Can be called with NULL baradd/barchar to test
int barcode39 (void *data, baradd_t * baradd, barchar_t * barchar, const char *value, int thin, int thick);
int barcodeitf (void *data, baradd_t * baradd, barchar_t * barchar, const char *value, int thin, int thick);
int barcode128 (void *data, baradd_t * baradd, barchar_t * barchar, const char *value);
int barcodeean (void *data, baradd_t * baradd, barchar_t * barchar, const char *value);
int barcodetelepen (void *data, baradd_t * baradd, barchar_t * barchar, int len, const char *value);    // ASCII
int barcodetelepennumeric (void *data, baradd_t * baradd, barchar_t * barchar, const char *value);      // Numeric

// Flags used in baradd and barchar
#define	BAR_BLACK	1       // Bar is logically black
#define	BAR_QUIET	2       // Quiet zone (start, end)
#define BAR_GUARD	4       // Guard bars
#define	BAR_DATA	8       // Bars that carry data
#define	BAR_BELOW	16      // baradd/barchar that are expected to have text printed below, e.g. typically shorter
#define	BAR_ABOVE	32      // baradd/barchar that are expected to have text printed above, e.g. typically shorter
#define	BAR_LEFT	64      // Called for barchar indicating that this text should be left aligned here
#define	BAR_RIGHT	128     // Called for barchar indicating that this text should be right aligned here
