#include "StdAfx.h"
#include "KFMImporter.h"
#include <kfm.h>
#include "gen/ControllerLink.h"
using namespace Niflib;

extern LPCTSTR AnimImportSection;

KFMImporter::KFMImporter(const TCHAR *Name,ImpInterface *I,Interface *GI, BOOL SuppressPrompts)
   : BaseClass()
{
   BaseInit(Name, I, GI, SuppressPrompts);
}

KFMImporter::KFMImporter(const vector<Ref<NiControllerSequence > >& roots, ImpInterface *I, Interface *GI, BOOL SuppressPrompts)
{
	i = I;
	gi = GI;
	suppressPrompts = SuppressPrompts;
	kf = roots;
	LoadIniSettings();
	Initialize();
}

KFMImporter::KFMImporter()
{
}

KFMImporter::~KFMImporter(void)
{
}

void KFMImporter::ReadBlocks()
{
   try
   {
      Kfm kfm;
	  string aname = T2AString(name);
	  string apath = T2AString(path);
      int ver = kfm.Read(aname);
      if (ver != VER_UNSUPPORTED)
      {
         TCHAR buffer[MAX_PATH];
         GetFullPathName(name.c_str(), MAX_PATH, buffer, nullptr);
         PathRemoveFileSpec(buffer);
         string nif_filename = apath + '\\' + kfm.nif_filename;
         if (_access(nif_filename.c_str(), 0) != -1)
            root = ReadNifTree(nif_filename);

         //root = kfm.MergeActions(string(buffer));
         BuildNodes();

         string last_file;
         vector<NiControllerSequenceRef> ctrllist;

         for ( vector<KfmAction>::iterator it = kfm.actions.begin(); it != kfm.actions.end(); it++ ) {
            string action_filename = apath + '\\' + it->action_filename;
            if (-1 != _access(action_filename.c_str(), 0)){
               if (action_filename != last_file) {
				   Niflib::NifInfo info;
                   ctrllist = DynamicCast<NiControllerSequence>(ReadNifList(action_filename, &info));
               }
               if (((*it).unk_int2 && (*it).unk_int2 <= ctrllist.size())) {
                  if (NiControllerSequenceRef ctrlSeq = ctrllist[(*it).unk_int2-1])
                     kf.push_back(ctrlSeq);
               } else if (!(*it).unk_int2 && ctrllist.size() == 1) {
                  if (NiControllerSequenceRef ctrlSeq = ctrllist[0])
                     kf.push_back(ctrlSeq);
               }
            }
         }
      }
   }
   catch (std::exception&)
   {
   }
   catch (...) 
   {
   }
}

bool KFMImporter::DoImport()
{
   bool ok = true;
   if (!suppressPrompts)
   {
      if (!ShowDialog())
         return true;
      ApplyAppSettings();
      SaveIniSettings();
   }

   // might check if blocks exist and if not go ahead and import nif.
   if (root)
   {
      int n = nodes.size();
      int m = 0;
      for (vector<NiNodeRef>::iterator itr = nodes.begin(), end = nodes.end(); itr != end; ++itr) {
         if (INode *p = FindNode(*itr))
            m++;
      }
      if (m != n) {
         BaseClass::DoImport();
      }
   }

   ClearAnimation();
   return ImportAnimation();
   //return BaseClass::DoImport();
}

void KFMImporter::SaveIniSettings()
{
   SetIniValue(AnimImportSection, TEXT("ClearAnimation"), clearAnimation);
   SetIniValue(AnimImportSection, TEXT("AddNoteTracks"), addNoteTracks);
   SetIniValue(AnimImportSection, TEXT("AddTimeTags"), addTimeTags);
}