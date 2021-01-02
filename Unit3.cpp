//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit3.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm3 *Form3;
//---------------------------------------------------------------------------
__fastcall TForm3::TForm3(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------


void __fastcall TForm3::Button1Click(TObject *Sender)
{
        ModalResult = mrOk;
}
//---------------------------------------------------------------------------

void __fastcall TForm3::Button2Click(TObject *Sender)
{
      ShellExecute(0, "open",  "StrftimeSpecifier.htm", "", "",  SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------

void __fastcall TForm3::Button3Click(TObject *Sender)
{
  SaveDialog1->Filter = "All files (*.*)|*.*";
  SaveDialog1->FilterIndex = 0;
  SaveDialog1->FileName = GCCodeListFilename->Text;
  if (SaveDialog1->Execute())
  {
      GCCodeListFilename->Clear();
      GCCodeListFilename->Text = SaveDialog1->Files->Strings[0].c_str();
  }
}
//---------------------------------------------------------------------------

int EditFile(char *s);
void __fastcall TForm3::Button4Click(TObject *Sender)
{
        EditFile(GCCodeListFilename->Text.c_str());
}
//---------------------------------------------------------------------------




