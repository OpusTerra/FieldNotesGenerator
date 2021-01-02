object Progress: TProgress
  Left = 473
  Top = 168
  BorderIcons = []
  BorderStyle = bsToolWindow
  Caption = 'Progess'
  ClientHeight = 123
  ClientWidth = 241
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnHide = FormHide
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 15
    Top = 32
    Width = 179
    Height = 13
    Caption = 'Running GPSBalbel "under the hood"'
  end
  object Image1: TImage
    Left = 200
    Top = 32
    Width = 33
    Height = 25
    Picture.Data = {
      07544269746D6170EE000000424DEE0000000000000076000000280000000F00
      00000F0000000100040000000000780000000000000000000000100000000000
      0000FFFFFF00DAF7FD0062CAF50041BAF2001AA4F200119CF200098FF2000681
      F2000372F20028AFF00000000000C0C0C0000000000000000000000000000000
      0000BBBBBAAAAABBBBB0BBBAA88888AABBB0BBA877777788ABB0BA8766AAAA77
      8AB0BA876A5556A67AB0A8764444555A78A0A6449994445A78A0A4993333945A
      78A0A4993223945678A0A44AAAA39AA678A0BA4933339AA78AB0BA4499994677
      8AB0BBA444994788ABB0BBBAA44468AABBB0BBBBBAAAAABBBBB0}
  end
  object ProgressBar1: TProgressBar
    Left = 46
    Top = 64
    Width = 150
    Height = 17
    Min = 0
    Max = 100
    TabOrder = 0
  end
end
