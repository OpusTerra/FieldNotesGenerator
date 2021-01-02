//---------------------------------------------------------------------------

#ifndef Unit3H
#define Unit3H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Mask.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TForm3 : public TForm
{
__published:	// IDE-managed Components
        TMaskEdit *Tolerance;
        TLabel *Label3;
        TMaskEdit *DoubleTrackDist;
        TLabel *Label4;
        TMaskEdit *DoubleTrackTime;
        TLabel *Label5;
        TMaskEdit *MinSegDist;
        TLabel *Label6;
        TCheckBox *DoubleTrackingEnabled;
        TCheckBox *OutTolerance;
        TCheckBox *AutoXchanger;
        TButton *Button1;
        TRadioGroup *GeocachingCodePosition;
        TMaskEdit *TotalCacheCounterInitialValue;
        TCheckBox *TotalCacheCounterDisplayEnabled;
        TLabel *Label1;
        TLabel *Label2;
        TCheckBox *TimeDisplayEnabled;
        TButton *Button2;
        TCheckBox *SaveIncrementalTotalCounter;
        TCheckBox *PartialCacheCounterDisplay;
        TEdit *PartialCounterFormat;
        TComboBox *TimeFormatSpecifierCombo;
        TComboBox *TotalCacheCounterFormatCombo;
        TCheckBox *GCCodeListGenerate;
        TEdit *GCCodeListFilename;
        TButton *Button3;
        TSaveDialog *SaveDialog1;
        TButton *Button4;
        TCheckBox *DNFChecking;
        TComboBox *DNFIcon;
        TLabel *Label7;
        TEdit *GPSBabelPath;
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall Button2Click(TObject *Sender);
        void __fastcall Button3Click(TObject *Sender);
        void __fastcall Button4Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TForm3(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm3 *Form3;
//---------------------------------------------------------------------------
#endif
