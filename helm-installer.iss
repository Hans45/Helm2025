[Setup]
AppName=Helm Synthesizer
AppVersion=0.9.0
AppPublisher=GitHub Copilot & Matt Tytel
AppPublisherURL=https://github.com/bepzi/helm
AppSupportURL=https://github.com/bepzi/helm/issues
AppUpdatesURL=https://github.com/bepzi/helm/releases
DefaultDirName={autopf}\Helm
DefaultGroupName=Helm Synthesizer
AllowNoIcons=yes
LicenseFile=COPYING
InfoBeforeFile=README.md
ShowLanguageDialog=yes
DisableWelcomePage=no
OutputDir=installer
OutputBaseFilename=helm-setup-{#SetupSetting("AppVersion")}
;SetupIconFile=images\helm_icon.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
PrivilegesRequired=lowest

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"

[Types]
Name: "full"; Description: "Installation compl�te"
Name: "custom"; Description: "Installation personnalis�e"; Flags: iscustom

[Components]
Name: "standalone"; Description: "Application standalone"; Types: full custom; Flags: fixed
Name: "vst3"; Description: "Plugin VST3"; Types: full custom
Name: "lv2"; Description: "Plugin LV2"; Types: full custom
Name: "patches"; Description: "Banques de patches factory"; Types: full custom; Flags: fixed
Name: "patches\factory"; Description: "Factory Presets (274 patches)"; Types: full custom; Flags: fixed
Name: "patches\newfactory"; Description: "New Factory Presets (25 patches - nouvelles formes d'ondes)"; Types: full custom; Flags: fixed

[Dirs]
Name: "{userappdata}\Helm"; Flags: uninsneveruninstall; Check: not IsAdminInstallMode
Name: "{userappdata}\Helm\Patches"; Flags: uninsneveruninstall; Check: not IsAdminInstallMode
Name: "{commondocs}\Helm"; Flags: uninsneveruninstall
Name: "{commondocs}\Helm\Patches"; Flags: uninsneveruninstall

[Files]
; Application standalone (autonome - aucun DLL externe requis)
Source: "build\HelmStandalone_artefacts\Release\helm.exe"; DestDir: "{app}"; Components: standalone; Flags: ignoreversion

; Plugin VST3
Source: "G:\git\helm\build\HelmPlugin_artefacts\Release\VST3\helm.vst3\Contents\x86_64-win\helm.vst3"; DestDir: "{commoncf}\VST3\Helm.vst3"; Components: vst3; Flags: ignoreversion

; Plugin LV2
Source: "build\HelmPlugin_artefacts\Release\LV2\Helm.lv2\*"; DestDir: "{commoncf}\LV2\Helm.lv2"; Components: lv2; Flags: ignoreversion recursesubdirs createallsubdirs

; Documentation et licence
Source: "README.md"; DestDir: "{commondocs}\Helm"; Flags: ignoreversion
Source: "COPYING"; DestDir: "{commondocs}\Helm"; Flags: ignoreversion
Source: "changelog"; DestDir: "{commondocs}\Helm"; Flags: ignoreversion
Source: "docs\helm_manual.pdf"; DestDir: "{commondocs}\Helm"; Flags: ignoreversion 

; Factory Presets (r�pertoire public pour les plugins)
Source: "patches\Factory Presets\*"; DestDir: "{commondocs}\Helm\Patches\Factory Presets"; Components: patches\factory; Flags: ignoreversion recursesubdirs createallsubdirs

; New Factory Presets (r�pertoire public pour les plugins)
Source: "patches\New Factory Presets\*"; DestDir: "{commondocs}\Helm\Patches\New Factory Presets"; Components: patches\newfactory; Flags: ignoreversion recursesubdirs createallsubdirs

; Factory Presets (r�pertoire utilisateur pour le standalone)
Source: "patches\Factory Presets\*"; DestDir: "{userdocs}\Helm\Patches\Factory Presets"; Components: patches\factory; Flags: ignoreversion recursesubdirs createallsubdirs; Check: not IsAdminInstallMode

; New Factory Presets (r�pertoire utilisateur pour le standalone)
Source: "patches\New Factory Presets\*"; DestDir: "{userdocs}\Helm\Patches\New Factory Presets"; Components: patches\newfactory; Flags: ignoreversion recursesubdirs createallsubdirs; Check: not IsAdminInstallMode

[Icons]
Name: "{group}\Helm Synthesizer"; Filename: "{app}\helm.exe"; Components: standalone
Name: "{group}\Helm Manual"; Filename: "{commondocs}\Helm\README.md"
Name: "{group}\Helm License"; Filename: "{commondocs}\Helm\COPYING"
Name: "{group}\Original Helm Manual"; Filename: "{commondocs}\Helm\helm_manual.pdf"
Name: "{group}\{cm:UninstallProgram,Helm}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Helm Synthesizer"; Filename: "{app}\helm.exe"; Components: standalone; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; Components: standalone

[Run]
Filename: "{app}\helm.exe"; Description: "{cm:LaunchProgram,Helm Synthesizer}"; Flags: nowait postinstall skipifsilent; Components: standalone

[Code]
var
  ComponentsPage: TWizardPage;
  StandaloneCheckBox: TNewCheckBox;
  VST3CheckBox: TNewCheckBox;
  LV2CheckBox: TNewCheckBox;
  FactoryPatchesCheckBox: TNewCheckBox;
  NewFactoryPatchesCheckBox: TNewCheckBox;

procedure InitializeWizard;
begin
  // Page personnalis�e pour les composants
  ComponentsPage := CreateCustomPage(wpSelectComponents,
    'S�lection des composants', 'Choisissez les composants � installer.');

  // Checkbox pour standalone
  StandaloneCheckBox := TNewCheckBox.Create(ComponentsPage);
  StandaloneCheckBox.Parent := ComponentsPage.Surface;
  StandaloneCheckBox.Caption := 'Application Standalone (helm.exe)';
  StandaloneCheckBox.Left := 0;
  StandaloneCheckBox.Top := 0;
  StandaloneCheckBox.Width := ComponentsPage.SurfaceWidth;
  StandaloneCheckBox.Height := ScaleY(17);
  StandaloneCheckBox.Checked := True;
  StandaloneCheckBox.Enabled := False; // Toujours install�

  // Checkbox pour VST3
  VST3CheckBox := TNewCheckBox.Create(ComponentsPage);
  VST3CheckBox.Parent := ComponentsPage.Surface;
  VST3CheckBox.Caption := 'Plugin VST3 (pour DAW compatibles VST3)';
  VST3CheckBox.Left := 0;
  VST3CheckBox.Top := StandaloneCheckBox.Top + StandaloneCheckBox.Height + ScaleY(8);
  VST3CheckBox.Width := ComponentsPage.SurfaceWidth;
  VST3CheckBox.Height := ScaleY(17);
  VST3CheckBox.Checked := True;

  // Checkbox pour LV2
  LV2CheckBox := TNewCheckBox.Create(ComponentsPage);
  LV2CheckBox.Parent := ComponentsPage.Surface;
  LV2CheckBox.Caption := 'Plugin LV2 (pour DAW compatibles LV2)';
  LV2CheckBox.Left := 0;
  LV2CheckBox.Top := VST3CheckBox.Top + VST3CheckBox.Height + ScaleY(8);
  LV2CheckBox.Width := ComponentsPage.SurfaceWidth;
  LV2CheckBox.Height := ScaleY(17);
  LV2CheckBox.Checked := True;

  // Checkbox pour Factory Presets
  FactoryPatchesCheckBox := TNewCheckBox.Create(ComponentsPage);
  FactoryPatchesCheckBox.Parent := ComponentsPage.Surface;
  FactoryPatchesCheckBox.Caption := 'Factory Presets (274 patches originaux)';
  FactoryPatchesCheckBox.Left := 0;
  FactoryPatchesCheckBox.Top := LV2CheckBox.Top + LV2CheckBox.Height + ScaleY(16);
  FactoryPatchesCheckBox.Width := ComponentsPage.SurfaceWidth;
  FactoryPatchesCheckBox.Height := ScaleY(17);
  FactoryPatchesCheckBox.Checked := True;
  FactoryPatchesCheckBox.Enabled := False; // Toujours install�

  // Checkbox pour New Factory Presets
  NewFactoryPatchesCheckBox := TNewCheckBox.Create(ComponentsPage);
  NewFactoryPatchesCheckBox.Parent := ComponentsPage.Surface;
  NewFactoryPatchesCheckBox.Caption := 'New Factory Presets (25 patches - nouvelles formes d' + #39 + 'ondes)';
  NewFactoryPatchesCheckBox.Left := 0;
  NewFactoryPatchesCheckBox.Top := FactoryPatchesCheckBox.Top + FactoryPatchesCheckBox.Height + ScaleY(8);
  NewFactoryPatchesCheckBox.Width := ComponentsPage.SurfaceWidth;
  NewFactoryPatchesCheckBox.Height := ScaleY(17);
  NewFactoryPatchesCheckBox.Checked := True;
  NewFactoryPatchesCheckBox.Enabled := False; // Toujours install�
end;

function ShouldSkipPage(PageID: Integer): Boolean;
begin
  // Skip la page des composants par d�faut car nous avons notre page personnalis�e
  Result := (PageID = wpSelectComponents);
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;

  if CurPageID = ComponentsPage.ID then
  begin
    // Mettre � jour les composants s�lectionn�s
    if StandaloneCheckBox.Checked then
      WizardSelectComponents('standalone');
    if VST3CheckBox.Checked then
      WizardSelectComponents('vst3');
    if LV2CheckBox.Checked then
      WizardSelectComponents('lv2');
    if FactoryPatchesCheckBox.Checked then
      WizardSelectComponents('patches\factory');
    if NewFactoryPatchesCheckBox.Checked then
      WizardSelectComponents('patches\newfactory');
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    // Copier les patches dans le r�pertoire utilisateur m�me en mode admin
    if IsAdminInstallMode then
    begin
      // En mode admin, installer aussi dans le profil utilisateur actuel pour le standalone
      if WizardIsComponentSelected('patches\factory') then
      begin
        if DirExists(ExpandConstant('{userdocs}')) then
        begin
          CreateDir(ExpandConstant('{userdocs}\Helm\Patches\Factory Presets'));
          if CopyFile(ExpandConstant('{commondocs}\Helm\Patches\Factory Presets\*'),
                     ExpandConstant('{userdocs}\Helm\Patches\Factory Presets\'), False) then
            Log('Factory Presets copi�s vers le profil utilisateur');
        end;
      end;

      if WizardIsComponentSelected('patches\newfactory') then
      begin
        if DirExists(ExpandConstant('{userdocs}')) then
        begin
          CreateDir(ExpandConstant('{userdocs}\Helm\Patches\New Factory Presets'));
          if CopyFile(ExpandConstant('{commondocs}\Helm\Patches\New Factory Presets\*'),
                     ExpandConstant('{userdocs}\Helm\Patches\New Factory Presets\'), False) then
            Log('New Factory Presets copi�s vers le profil utilisateur');
        end;
      end;
    end;
  end;
end;

[Messages]
english.WelcomeLabel2=This will install [name/ver] on your computer.%n%nHelm is a free, cross-platform, polyphonic synthesizer featuring 21 different waveforms, powerful modulation, and professional effects.%n%nThis version includes 8 new waveforms: Pulse variations, hybrid waves, and textured oscillations for expanded sonic possibilities.
french.WelcomeLabel2=Ceci installera [name/ver] sur votre ordinateur.%n%nHelm est un synth�tiseur polyphonique gratuit et multiplateforme avec 21 formes d'ondes diff�rentes, une modulation puissante et des effets professionnels.%n%nCette version inclut 8 nouvelles formes d'ondes : variations de pulse, ondes hybrides et oscillations textur�es pour des possibilit�s sonores �tendues.

; Messages personnalis�s pour la licence
english.LicenseLabel=Please read the following License Agreement. Helm is distributed under the GNU General Public License v3.
french.LicenseLabel=Veuillez lire le contrat de licence suivant. Helm est distribu� sous la licence GNU General Public License v3.
english.LicenseLabel3=If you accept the terms of the agreement, click I accept the agreement to continue. You must accept the agreement to install [name].
french.LicenseLabel3=Si vous acceptez les termes du contrat, cliquez sur J'accepte le contrat pour continuer. Vous devez accepter le contrat pour installer [name].
english.LicenseAccepted=&I accept the agreement
french.LicenseAccepted=&J'accepte le contrat
english.LicenseNotAccepted=I &do not accept the agreement
french.LicenseNotAccepted=Je &n'accepte pas le contrat

[Registry]
; Association de fichiers .helm
Root: HKCR; Subkey: ".helm"; ValueType: string; ValueName: ""; ValueData: "HelmPatch"; Flags: uninsdeletevalue; Components: standalone
Root: HKCR; Subkey: "HelmPatch"; ValueType: string; ValueName: ""; ValueData: "Helm Synthesizer Patch"; Flags: uninsdeletekey; Components: standalone
Root: HKCR; Subkey: "HelmPatch\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\helm.exe,0"; Components: standalone
Root: HKCR; Subkey: "HelmPatch\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\helm.exe"" ""%1"""; Components: standalone

[UninstallDelete]
Type: filesandordirs; Name: "{app}"