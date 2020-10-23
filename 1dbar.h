// 1D barcode generators
// (c) 2019 Adrian Kennard

// Callback functions (return width added)
typedef void baradd_t (void *data, int w, int flags);   // function that adds next bar
typedef void barchar_t (void *data, const char *text, int n, int dx, int w, int cw, int flags); // function that adds text (n chars of text, at offset dx w units, font based on cw character width

// Barcode generation functions - these call the callback functions
// baradd is called in turn for each bar from left to right starting with quiet zone
// barchar is called for characters or strings, interleaved with the bars
typedef struct
{
   char *value;                 // The barcode content
   unsigned char len;           // If needed for type of barcode
   unsigned char thin;          // If needed for type of barcode
   unsigned char thick;         // If needed for type of barcode
   baradd_t *baradd;            // Function for adding bars
   barchar_t *barchar;          // Function for adding characters
   void *data;                  // Data to pass to function
   unsigned char numeric:1;     // Flag alternative numeric coding
} barcode_t;
void barcode39_opts (barcode_t);
#define	barcode39(...) barcode39_opts((barcode_t){__VA_ARGS__})
void barcodeitf_opts (barcode_t);
#define	barcodeitf(...) barcodeitf_opts((barcode_t){__VA_ARGS__})
void barcode128_opts (barcode_t);
#define	barcode128(...) barcode128_opts((barcode_t){__VA_ARGS__})
void barcodeean_opts (barcode_t);
#define	barcodeean(...) barcodeean_opts((barcode_t){__VA_ARGS__})
void barcodetelepen_opts (barcode_t);
#define	barcodetelepen(...) barcodetelepen_opts((barcode_t){__VA_ARGS__})

// Flags used in baradd and barchar
#define	BAR_BLACK	1       // Bar is logically black
#define	BAR_QUIET	2       // Quiet zone (start, end)
#define BAR_GUARD	4       // Guard bars
#define	BAR_DATA	8       // Bars that carry data
#define	BAR_BELOW	16      // baradd/barchar that are expected to have text printed below, e.g. typically shorter
#define	BAR_ABOVE	32      // baradd/barchar that are expected to have text printed above, e.g. typically shorter
#define	BAR_LEFT	64      // Called for barchar indicating that this text should be left aligned here
#define	BAR_RIGHT	128     // Called for barchar indicating that this text should be right aligned here
