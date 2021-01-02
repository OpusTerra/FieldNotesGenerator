object Form3: TForm3
  Left = 284
  Top = 252
  AutoSize = True
  BorderStyle = bsDialog
  BorderWidth = 15
  Caption = 'Parameters'
  ClientHeight = 433
  ClientWidth = 697
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label3: TLabel
    Left = 51
    Top = 8
    Width = 121
    Height = 13
    Caption = 'Out of Tolerance (Meters)'
  end
  object Label4: TLabel
    Left = 51
    Top = 40
    Width = 202
    Height = 13
    Caption = 'Double tracking minimum distance (Meters)'
  end
  object Label5: TLabel
    Left = 51
    Top = 72
    Width = 207
    Height = 13
    Caption = 'Time tolerance for double tracking (Minutes)'
  end
  object Label6: TLabel
    Left = 51
    Top = 104
    Width = 223
    Height = 13
    Caption = 'Minimal Google Map segment distance (Meters)'
  end
  object Label1: TLabel
    Left = 384
    Top = 104
    Width = 157
    Height = 13
    Caption = 'Total Found Cache Counter'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label2: TLabel
    Left = 384
    Top = 224
    Width = 71
    Height = 13
    Caption = 'Time display'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label7: TLabel
    Left = 0
    Top = 376
    Width = 77
    Height = 13
    Caption = 'GPS Babel Path'
  end
  object GeocachingCodePosition: TRadioGroup
    Left = 0
    Top = 136
    Width = 361
    Height = 129
    Caption = 'Geocaching Code Position'
    ItemIndex = 0
    Items.Strings = (
      'Start of Description field with GC prefix'
      'Start of Description field without GC prefix'
      'Start of Waypoint Name field with GC prefix'
      'Start of Waypoint Name field without GC prefix')
    TabOrder = 8
  end
  object Tolerance: TMaskEdit
    Left = 0
    Top = 0
    Width = 33
    Height = 21
    Hint = 'If distance if above this length, a warning will be issued'
    EditMask = '999;1;_'
    MaxLength = 3
    ParentShowHint = False
    ShowHint = True
    TabOrder = 0
    Text = '   '
  end
  object DoubleTrackDist: TMaskEdit
    Left = 0
    Top = 32
    Width = 33
    Height = 21
    Hint = 'Distance should be below this length'
    EditMask = '999;1;_'
    MaxLength = 3
    ParentShowHint = False
    ShowHint = True
    TabOrder = 1
    Text = '   '
  end
  object DoubleTrackTime: TMaskEdit
    Left = 0
    Top = 64
    Width = 33
    Height = 21
    Hint = 'Time found should be above this period of time'
    EditMask = '999;1;_'
    MaxLength = 3
    ParentShowHint = False
    ShowHint = True
    TabOrder = 2
    Text = '   '
  end
  object MinSegDist: TMaskEdit
    Left = 0
    Top = 96
    Width = 33
    Height = 21
    Hint = 'Used to limit google map trace segment plotting'
    EditMask = '999;1;_'
    MaxLength = 3
    ParentShowHint = False
    ShowHint = True
    TabOrder = 3
    Text = '   '
  end
  object DoubleTrackingEnabled: TCheckBox
    Left = 312
    Top = 16
    Width = 185
    Height = 17
    Caption = 'Do not check double tracking'
    TabOrder = 4
  end
  object OutTolerance: TCheckBox
    Left = 312
    Top = 40
    Width = 321
    Height = 17
    Caption = 'Include out of tolerance FOUND geocaches in Field Notes file'
    TabOrder = 5
  end
  object AutoXchanger: TCheckBox
    Left = 312
    Top = 64
    Width = 361
    Height = 17
    Caption = 
      'Auto exchange tracking values based on min distance and min spee' +
      'd'
    TabOrder = 6
  end
  object Button1: TButton
    Left = 312
    Top = 408
    Width = 75
    Height = 25
    Caption = 'Done'
    TabOrder = 7
    OnClick = Button1Click
  end
  object TotalCacheCounterDisplayEnabled: TCheckBox
    Left = 384
    Top = 120
    Width = 25
    Height = 17
    Hint = 'Enable Cache Counter display'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 10
  end
  object TotalCacheCounterInitialValue: TMaskEdit
    Left = 408
    Top = 120
    Width = 65
    Height = 21
    Hint = 'Enter Starting Value'
    EditMask = '!99999999;1;_'
    MaxLength = 8
    ParentShowHint = False
    ShowHint = True
    TabOrder = 9
    Text = '        '
  end
  object TimeDisplayEnabled: TCheckBox
    Left = 384
    Top = 240
    Width = 25
    Height = 17
    Hint = 'Enabled'
    TabOrder = 11
  end
  object Button2: TButton
    Left = 512
    Top = 240
    Width = 105
    Height = 25
    Caption = 'More time formats'
    TabOrder = 12
    OnClick = Button2Click
  end
  object SaveIncrementalTotalCounter: TCheckBox
    Left = 384
    Top = 144
    Width = 249
    Height = 17
    Caption = 'Save Incremented Total counter'
    TabOrder = 13
  end
  object PartialCacheCounterDisplay: TCheckBox
    Left = 384
    Top = 176
    Width = 225
    Height = 17
    Caption = 'Partial Found Cache Counter'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 14
  end
  object PartialCounterFormat: TEdit
    Left = 592
    Top = 176
    Width = 105
    Height = 21
    Hint = 'Format Specifier'
    TabOrder = 15
    Text = 'PartialCounterFormat'
  end
  object TimeFormatSpecifierCombo: TComboBox
    Left = 408
    Top = 240
    Width = 89
    Height = 21
    ItemHeight = 13
    TabOrder = 16
    Text = 'TimeFormatSpecifierCombo'
    Items.Strings = (
      '%Hh%M'
      '%H:%M:%S '
      '%I:%M%p'
      '%A %I:%M%p')
  end
  object TotalCacheCounterFormatCombo: TComboBox
    Left = 488
    Top = 120
    Width = 105
    Height = 21
    ItemHeight = 13
    TabOrder = 17
    Text = 'TotalCacheCounterFormatCombo'
    Items.Strings = (
      '#%d'
      '%d'
      '(#%d)'
      '(Cache #%d)'
      '(Cache %d)')
  end
  object GCCodeListGenerate: TCheckBox
    Left = 0
    Top = 280
    Width = 257
    Height = 17
    Caption = 'Only just extract all GC codes for GSAK usage'
    TabOrder = 18
  end
  object GCCodeListFilename: TEdit
    Left = 328
    Top = 280
    Width = 289
    Height = 21
    Hint = 'GC Code List Filename'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 19
  end
  object Button3: TButton
    Left = 272
    Top = 280
    Width = 49
    Height = 25
    Caption = 'Save as:'
    TabOrder = 20
    OnClick = Button3Click
  end
  object Button4: TButton
    Left = 616
    Top = 280
    Width = 57
    Height = 25
    Caption = 'Wordpad'
    TabOrder = 21
    OnClick = Button4Click
  end
  object DNFChecking: TCheckBox
    Left = 0
    Top = 328
    Width = 217
    Height = 17
    Hint = 
      'With your GPS or within Mapsource, before processing, change for' +
      ' this icon to  indicate a DNF type'
    Caption = 'Add DNF checking using this icon:'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 22
  end
  object DNFIcon: TComboBox
    Left = 232
    Top = 328
    Width = 161
    Height = 21
    Hint = 
      'With your GPS or within Mapsource, before processing, change for' +
      ' this icon to  indicate a DNF type'
    Style = csDropDownList
    ItemHeight = 13
    ParentShowHint = False
    ShowHint = True
    TabOrder = 23
    Items.Strings = (
      'Airport'
      'Amusement Park'
      'Ball Park'
      'Bank'
      'Bar'
      'Beach'
      'Bell'
      'Boat Ramp'
      'Bowling'
      'Bridge'
      'Building'
      'Campground'
      'Car'
      'Car Rental'
      'Car Repair'
      'Cemetery'
      'Church'
      'Circle with X'
      'City (Capitol)'
      'City (Large)'
      'City (Medium)'
      'City (Small)'
      'Civil'
      'Contact, Afro'
      'Contact, Alien'
      'Contact, Ball Cap'
      'Contact, Big Ears'
      'Contact, Biker'
      'Contact, Bug'
      'Contact, Cat'
      'Contact, Dog'
      'Contact, Dreadlocks'
      'Contact, Female1'
      'Contact, Female2'
      'Contact, Female3'
      'Contact, Goatee'
      'Contact, Kung-Fu'
      'Contact, Pig'
      'Contact, Pirate'
      'Contact, Ranger'
      'Contact, Smiley'
      'Contact, Spike'
      'Contact, Sumo'
      'Controlled Area'
      'Convenience Store'
      'Crossing'
      'Dam'
      'Danger Area'
      'Department Store'
      'Diver Down Flag 1'
      'Diver Down Flag 2'
      'Drinking Water'
      'Exit'
      'Fast Food'
      'Fishing Area'
      'Fitness Center'
      'Flag'
      'Forest'
      'Gas Station'
      'Geocache'
      'Geocache Found'
      'Ghost Town'
      'Glider Area'
      'Golf Course'
      'Green Diamond'
      'Green Square'
      'Heliport'
      'Horn'
      'Hunting Area'
      'Information'
      'Levee'
      'Light'
      'Live Theater'
      'Lodging'
      'Man Overboard'
      'Marina'
      'Medical Facility'
      'Mile Marker'
      'Military'
      'Mine'
      'Movie Theater'
      'Museum'
      'Navaid, Amber'
      'Navaid, Black'
      'Navaid, Blue'
      'Navaid, Green'
      'Navaid, Green/Red'
      'Navaid, Green/White'
      'Navaid, Orange'
      'Navaid, Red'
      'Navaid, Red/Green'
      'Navaid, Red/White'
      'Navaid, Violet'
      'Navaid, White'
      'Navaid, White/Green'
      'Navaid, White/Red'
      'Oil Field'
      'Parachute Area'
      'Park'
      'Parking Area'
      'Pharmacy'
      'Picnic Area'
      'Pizza'
      'Post Office'
      'Private Field'
      'Radio Beacon'
      'Red Diamond'
      'Red Square'
      'Residence'
      'Restaurant'
      'Restricted Area'
      'Restroom'
      'RV Park'
      'Scales'
      'Scenic Area'
      'School'
      'Seaplane Base'
      'Shipwreck'
      'Shopping Center'
      'Short Tower'
      'Shower'
      'Skiing Area'
      'Skull and Crossbones'
      'Soft Field'
      'Stadium'
      'Summit'
      'Swimming Area'
      'Tall Tower'
      'Telephone'
      'Toll Booth'
      'TracBack Point'
      'Trail Head'
      'Truck Stop'
      'Tunnel'
      'Ultralight Area'
      'Water Hydrant'
      'Waypoint'
      'White Buoy'
      'White Dot'
      'Zoo'
      'Custom 0 '
      'Custom 1 '
      'Custom 2 '
      'Custom 3 '
      'Custom 4 '
      'Custom 5 '
      'Custom 6 '
      'Custom 7 '
      'Custom 8 '
      'Custom 9 '
      'Custom 10'
      'Custom 11'
      'Custom 12'
      'Custom 13'
      'Custom 14'
      'Custom 15'
      'Custom 16'
      'Custom 17'
      'Custom 18'
      'Custom 19'
      'Custom 20'
      'Custom 21'
      'Custom 22'
      'Custom 23')
  end
  object GPSBabelPath: TEdit
    Left = 96
    Top = 368
    Width = 497
    Height = 21
    TabOrder = 24
    Text = 'GPSBabelPath'
  end
  object SaveDialog1: TSaveDialog
    Left = 384
    Top = 280
  end
end
