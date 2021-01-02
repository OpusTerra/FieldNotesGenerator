//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include <Mask.hpp>
#include <ExtCtrls.hpp>
#include <jpeg.hpp>
#include <Menus.hpp>
#include <Buttons.hpp>
#include <Graphics.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
        TOpenDialog *OpenDialog1;
        TEdit *GPXFile;
        TButton *Button1;
        TLabel *Label11;
        TListView *ListView1;
        TLabel *Label2;
        TPopupMenu *PopupMenu1;
        TMenuItem *ViewinGSAK1;
        TMenuItem *ViewatGCCOM1;
        TButton *Button3;
        TButton *Button4;
        TSpeedButton *SpeedButton1;
        TMenuItem *ViewOffline1;
        TSaveDialog *SaveDialog1;
        TButton *Button5;
        TButton *Button7;
        TButton *Button8;
        TButton *Button9;
        TLabel *OutFileName;
        TButton *GMapViewAll;
        TMenuItem *Exchangetrackingvalues1;
        TMenuItem *ViewinGoogleMap1;
        TMainMenu *MainMenu1;
        TMenuItem *Exit1;
        TMenuItem *Exit2;
        TMenuItem *Options1;
        TBitBtn *Process;
        TButton *Button2;
        TLabel *Label1;
        TMenuItem *Help1;
        TMenuItem *FRancais1;
        TMenuItem *English1;
        TSpeedButton *SpeedButton2;
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall ProcessClick(TObject *Sender);
        void __fastcall ViewinGSAK1Click(TObject *Sender);
        void __fastcall ViewatGCCOM1Click(TObject *Sender);
        void __fastcall LogCacheonline1Click(TObject *Sender);
        void __fastcall Button3Click(TObject *Sender);
        void __fastcall Button4Click(TObject *Sender);
        void __fastcall SpeedButton1Click(TObject *Sender);
        void __fastcall ViewOffline1Click(TObject *Sender);
        void __fastcall BabelCallingClick(TObject *Sender);
        void __fastcall Button5Click(TObject *Sender);
        void __fastcall ListView1CustomDrawItem(TCustomListView *Sender,
          TListItem *Item, TCustomDrawState State, bool &DefaultDraw);
        void __fastcall ListView1ColumnClick(TObject *Sender,
          TListColumn *Column);
        void __fastcall ListView1Compare(TObject *Sender, TListItem *Item1,
          TListItem *Item2, int Data, int &Compare);
        void __fastcall Button7Click(TObject *Sender);
        void __fastcall Button8Click(TObject *Sender);
        void __fastcall Button9Click(TObject *Sender);
        void __fastcall Button6Click(TObject *Sender);
        void __fastcall GMapViewAllClick(TObject *Sender);
        void __fastcall ViewinGoogleMap1Click(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall Options1Click(TObject *Sender);
        void __fastcall Exit2Click(TObject *Sender);
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall Label11Click(TObject *Sender);
        void __fastcall FRancais1Click(TObject *Sender);
        void __fastcall English1Click(TObject *Sender);
        void __fastcall SpeedButton2Click(TObject *Sender);
private:	// User declarations
        int ColumnToSort;

public:		// User declarations
        __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
