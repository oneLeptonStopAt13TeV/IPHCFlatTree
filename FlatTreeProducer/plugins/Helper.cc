#include "IPHCFlatTree/FlatTreeProducer/interface/Helper.hh"

#include "TMath.h"

float GetDeltaR(float eta1,float phi1,float eta2,float phi2)
{
   float DeltaPhi = TMath::Abs(phi2 - phi1);
      if (DeltaPhi > 3.141593 ) DeltaPhi -= 2.*3.141593;
   return TMath::Sqrt( (eta2-eta1)*(eta2-eta1) + DeltaPhi*DeltaPhi );
}

// https://github.com/manuelfs/CfANtupler/blob/master/minicfa/interface/miniAdHocNTupler.h#L203
double getPFIsolation(edm::Handle<pat::PackedCandidateCollection> pfcands,
		      const reco::Candidate* ptcl,
		      double r_iso_min, double r_iso_max, double kt_scale,
		      bool use_pfweight, bool charged_only) 
{
   if (ptcl->pt()<5.) return 99999.;
   double deadcone_nh(0.), deadcone_ch(0.), deadcone_ph(0.), deadcone_pu(0.);
   if(ptcl->isElectron()) 
     {	
	if (fabs(ptcl->eta())>1.479) 
	  {
	     deadcone_ch = 0.015; deadcone_pu = 0.015; deadcone_ph = 0.08;
	  }	
     }
    else if(ptcl->isMuon()) 
     {	
	deadcone_ch = 0.0001; deadcone_pu = 0.01; deadcone_ph = 0.01;deadcone_nh = 0.01;
     }
    else 
     {	
	//deadcone_ch = 0.0001; deadcone_pu = 0.01; deadcone_ph = 0.01;deadcone_nh = 0.01; // maybe use muon cones??
     }
   
   double iso_nh(0.); double iso_ch(0.);
   double iso_ph(0.); double iso_pu(0.);
   double ptThresh(0.5);
   if(ptcl->isElectron()) ptThresh = 0;
   double r_iso = std::max(r_iso_min,std::min(r_iso_max, kt_scale/ptcl->pt()));
   for (const pat::PackedCandidate &pfc : *pfcands)
     {	
	if (abs(pfc.pdgId())<7) continue;
	double dr = deltaR(pfc, *ptcl);
	if (dr > r_iso) continue;
	////////////////// NEUTRALS /////////////////////////
	if (pfc.charge()==0)
	  {	     
	     if (pfc.pt()>ptThresh) 
	       {		  
		  double wpf(1.);
		  if (use_pfweight)
		    {		       
		       double wpv(0.), wpu(0.);
		       for (const pat::PackedCandidate &jpfc : *pfcands) 
			 {			    
			    double jdr = deltaR(pfc, jpfc);
			    if (pfc.charge()!=0 || jdr<0.00001) continue;
			    double jpt = jpfc.pt();
			    if (pfc.fromPV()>1) wpv *= jpt/jdr;
			    else wpu *= jpt/jdr;
			 }
		       
		       wpv = log(wpv);
		       wpu = log(wpu);
		       wpf = wpv/(wpv+wpu);
		    }
		  
		  /////////// PHOTONS ////////////
		  if (abs(pfc.pdgId())==22) 
		    {		       
		       if(dr < deadcone_ph) continue;
		       iso_ph += wpf*pfc.pt();
		       /////////// NEUTRAL HADRONS ////////////
		    }
		   else if (abs(pfc.pdgId())==130) 
		    {		       
		       if(dr < deadcone_nh) continue;
		       iso_nh += wpf*pfc.pt();
		    }		  
	       }	     
	     ////////////////// CHARGED from PV /////////////////////////
	  }
	 else if (pfc.fromPV()>1)
	  {	     
	     if (abs(pfc.pdgId())==211) 
	       {		  
		  if(dr < deadcone_ch) continue;
		  iso_ch += pfc.pt();
	       }
	     
	     ////////////////// CHARGED from PU /////////////////////////
	  }
	 else 
	  {	     
	     if (pfc.pt()>ptThresh)
	       {		  
		  if(dr < deadcone_pu) continue;
		  iso_pu += pfc.pt();
	       }	     
	  }	
     }
   
   double iso(0.);
   if (charged_only)
     {	
	iso = iso_ch;
     }
    else 
     {	
	iso = iso_ph + iso_nh;
	if (!use_pfweight) iso -= 0.5*iso_pu;
	if (iso>0) iso += iso_ch;
	else iso = iso_ch;
     }
   
   iso = iso/ptcl->pt();
   return iso;
}

   
