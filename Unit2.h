//---------------------------------------------------------------------------

#ifndef Unit2H
#define Unit2H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
//---------------------------------------------------------------------------
class TProgress : public TForm
{
__published:	// IDE-managed Components
        TProgressBar *ProgressBar1;
        TLabel *Label1;
        TImage *Image1;
        void __fastcall FormShow(TObject *Sender);
        void __fastcall FormHide(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TProgress(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TProgress *Progress;
//---------------------------------------------------------------------------
#endif
