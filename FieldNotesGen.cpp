//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USERES("FieldNotesGen.res");
USEFORM("Unit1.cpp", Form1);
USEUNIT("Coord.cpp");
USEFORM("Unit2.cpp", Progress);
USEUNIT("File1.c");
USEFORM("Unit3.cpp", Form3);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->CreateForm(__classid(TForm1), &Form1);
                 Application->CreateForm(__classid(TProgress), &Progress);
                 Application->CreateForm(__classid(TForm3), &Form3);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------
