// system include files
#include <atomic>
#include <memory>
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDFilter.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/Common/interface/TriggerNames.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"

#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "Calibration/IsolatedParticles/interface/CaloPropagateTrack.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"

//#define EDM_ML_DEBUG
//
// class declaration
//

namespace AlCaHBHEMuons {
  struct Counters {
    Counters() : nAll_(0), nGood_(0), nFinal_(0) {}
    mutable std::atomic<unsigned int> nAll_, nGood_, nFinal_;
  };
}  // namespace AlCaHBHEMuons

class AlCaHBHEMuonFilter : public edm::stream::EDFilter<edm::GlobalCache<AlCaHBHEMuons::Counters> > {
public:
  explicit AlCaHBHEMuonFilter(edm::ParameterSet const&, const AlCaHBHEMuons::Counters* count);
  ~AlCaHBHEMuonFilter() override;

  static std::unique_ptr<AlCaHBHEMuons::Counters> initializeGlobalCache(edm::ParameterSet const&) {
    return std::make_unique<AlCaHBHEMuons::Counters>();
  }

  bool filter(edm::Event&, edm::EventSetup const&) override;
  void endStream() override;
  static void globalEndJob(const AlCaHBHEMuons::Counters* counters);
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginRun(edm::Run const&, edm::EventSetup const&) override;
  void endRun(edm::Run const&, edm::EventSetup const&) override;

  // ----------member data ---------------------------
  HLTConfigProvider hltConfig_;
  std::vector<std::string> trigNames_, HLTNames_;
  std::string processName_;
  bool pfCut_;
  double trackIsoCut_, caloIsoCut_, pfIsoCut_;
  int preScale_;
  unsigned int nRun_, nAll_, nGood_, nFinal_;
  edm::InputTag triggerResults_, labelMuon_;
  edm::EDGetTokenT<trigger::TriggerEvent> tok_trigEvt;
  edm::EDGetTokenT<edm::TriggerResults> tok_trigRes_;
  edm::EDGetTokenT<reco::MuonCollection> tok_Muon_;
  edm::ESGetToken<CaloGeometry, CaloGeometryRecord> tok_geom_;
  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> tok_magField_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
AlCaHBHEMuonFilter::AlCaHBHEMuonFilter(edm::ParameterSet const& iConfig, const AlCaHBHEMuons::Counters* count)
    : nRun_(0), nAll_(0), nGood_(0), nFinal_(0) {
  //now do what ever initialization is needed
  trigNames_ = iConfig.getParameter<std::vector<std::string> >("Triggers");
  processName_ = iConfig.getParameter<std::string>("ProcessName");
  triggerResults_ = iConfig.getParameter<edm::InputTag>("TriggerResultLabel");
  labelMuon_ = iConfig.getParameter<edm::InputTag>("MuonLabel");
  pfCut_ = iConfig.getParameter<bool>("PFCut");
  pfIsoCut_ = iConfig.getParameter<double>("PFIsolationCut");
  trackIsoCut_ = iConfig.getParameter<double>("TrackIsolationCut");
  caloIsoCut_ = iConfig.getParameter<double>("CaloIsolationCut");
  preScale_ = iConfig.getParameter<int>("PreScale");
  if (preScale_ < 1)
    preScale_ = 1;

  // define tokens for access
  tok_trigRes_ = consumes<edm::TriggerResults>(triggerResults_);
  tok_Muon_ = consumes<reco::MuonCollection>(labelMuon_);

  tok_geom_ = esConsumes<CaloGeometry, CaloGeometryRecord>();
  tok_magField_ = esConsumes<MagneticField, IdealMagneticFieldRecord>();

  edm::LogVerbatim("HBHEMuon") << "Parameters read from config file \n"
                               << "Process " << processName_ << "  Prescale " << preScale_ << "  Isolation Cuts "
                               << trackIsoCut_ << ":" << caloIsoCut_ << "\n";
  for (unsigned int k = 0; k < trigNames_.size(); ++k)
    edm::LogVerbatim("HBHEMuon") << "Trigger[" << k << "] " << trigNames_[k] << "\n";
}  // AlCaHBHEMuonFilter::AlCaHBHEMuonFilter  constructor

AlCaHBHEMuonFilter::~AlCaHBHEMuonFilter() {}

//
// member functions
//

// ------------ method called on each new Event  ------------
bool AlCaHBHEMuonFilter::filter(edm::Event& iEvent, edm::EventSetup const& iSetup) {
  bool accept(false);
  ++nAll_;
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HBHEMuon") << "AlCaHBHEMuonFilter::Run " << iEvent.id().run() << " Event " << iEvent.id().event()
                               << " Luminosity " << iEvent.luminosityBlock() << " Bunch " << iEvent.bunchCrossing()
                               << std::endl;
#endif
  //Step1: Find if the event passes one of the chosen triggers
  bool ok(false);
  /////////////////////////////TriggerResults
  if (trigNames_.empty()) {
    ok = true;
  } else {
    edm::Handle<edm::TriggerResults> triggerResults;
    iEvent.getByToken(tok_trigRes_, triggerResults);
    if (triggerResults.isValid()) {
      std::vector<std::string> modules;
      const edm::TriggerNames& triggerNames = iEvent.triggerNames(*triggerResults);
      const std::vector<std::string>& triggerNames_ = triggerNames.triggerNames();
      for (unsigned int iHLT = 0; iHLT < triggerResults->size(); iHLT++) {
        int hlt = triggerResults->accept(iHLT);
        for (unsigned int i = 0; i < trigNames_.size(); ++i) {
          if (triggerNames_[iHLT].find(trigNames_[i]) != std::string::npos) {
            if (hlt > 0) {
              ok = true;
            }
#ifdef EDM_ML_DEBUG
            edm::LogVerbatim("HBHEMuon") << "AlCaHBHEMuonFilter::Trigger " << triggerNames_[iHLT] << " Flag " << hlt
                                         << ":" << ok << std::endl;
#endif
          }
        }
      }
    }
  }
  if (ok) {
    //Step2: Get geometry/B-field information
    const MagneticField* bField = &(iSetup.getData(tok_magField_));
    const CaloGeometry* geo = &(iSetup.getData(tok_geom_));

    // Relevant blocks from iEvent
    edm::Handle<reco::MuonCollection> _Muon;
    iEvent.getByToken(tok_Muon_, _Muon);
#ifdef EDM_ML_DEBUG
    edm::LogVerbatim("HBHEMuon") << "AlCaHBHEMuonFilter::Muon Handle " << _Muon.isValid() << std::endl;
#endif
    if (_Muon.isValid()) {
      for (reco::MuonCollection::const_iterator RecMuon = _Muon->begin(); RecMuon != _Muon->end(); ++RecMuon) {
#ifdef EDM_ML_DEBUG
        edm::LogVerbatim("HBHEMuon") << "AlCaHBHEMuonFilter::Muon:Track " << RecMuon->track().isNonnull()
                                     << " innerTrack " << RecMuon->innerTrack().isNonnull() << " outerTrack "
                                     << RecMuon->outerTrack().isNonnull() << " globalTrack "
                                     << RecMuon->globalTrack().isNonnull() << std::endl;
#endif
        if ((RecMuon->track().isNonnull()) && (RecMuon->innerTrack().isNonnull()) &&
            (RecMuon->outerTrack().isNonnull()) && (RecMuon->globalTrack().isNonnull())) {
          const reco::Track* pTrack = (RecMuon->innerTrack()).get();
          spr::propagatedTrackID trackID = spr::propagateCALO(pTrack, geo, bField, false);
#ifdef EDM_ML_DEBUG
          edm::LogVerbatim("HBHEMuon") << "AlCaHBHEMuonFilter::Propagate: ECAL " << trackID.okECAL << " to HCAL "
                                       << trackID.okHCAL << std::endl;
#endif
          double trackIso = RecMuon->isolationR03().sumPt;
          double caloIso = RecMuon->isolationR03().emEt + RecMuon->isolationR03().hadEt;
          double isolR04 =
              ((RecMuon->pfIsolationR04().sumChargedHadronPt +
                std::max(0.,
                         RecMuon->pfIsolationR04().sumNeutralHadronEt + RecMuon->pfIsolationR04().sumPhotonEt -
                             (0.5 * RecMuon->pfIsolationR04().sumPUPt))) /
               RecMuon->pt());
          bool isoCut = (pfCut_) ? (isolR04 < pfIsoCut_) : ((trackIso < trackIsoCut_) && (caloIso < caloIsoCut_));
          if ((trackID.okECAL) && (trackID.okHCAL) && isoCut) {
            accept = true;
            break;
          }
        }
      }
    }
  }
  // Step 4:  Return the acceptance flag
  if (accept) {
    ++nGood_;
    if (((nGood_ - 1) % preScale_) != 0) {
      accept = false;
    } else {
      ++nFinal_;
    }
  }
  return accept;

}  // AlCaHBHEMuonFilter::filter

// ------------ method called once each job just after ending the event loop  ------------
void AlCaHBHEMuonFilter::endStream() {
  globalCache()->nAll_ += nAll_;
  globalCache()->nGood_ += nGood_;
  globalCache()->nFinal_ += nFinal_;
}

void AlCaHBHEMuonFilter::globalEndJob(const AlCaHBHEMuons::Counters* count) {
  edm::LogVerbatim("HBHEMuon") << "Selects " << count->nFinal_ << " out of " << count->nGood_ << " good events out of "
                               << count->nAll_ << " total # of events\n";
}

// ------------ method called when starting to processes a run  ------------
void AlCaHBHEMuonFilter::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {
  bool changed(false);
  bool flag = hltConfig_.init(iRun, iSetup, processName_, changed);
  edm::LogVerbatim("HBHEMuon") << "Run[" << nRun_ << "] " << iRun.run() << " hltconfig.init " << flag << std::endl;
}

// ------------ method called when ending the processing of a run  ------------
void AlCaHBHEMuonFilter::endRun(edm::Run const& iRun, edm::EventSetup const&) {
  edm::LogVerbatim("HBHEMuon") << "endRun[" << nRun_ << "] " << iRun.run() << "\n";
  nRun_++;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void AlCaHBHEMuonFilter::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  std::vector<std::string> triggers = {"HLT_IsoMu", "HLT_Mu"};
  desc.add<std::string>("ProcessName", "HLT");
  desc.add<edm::InputTag>("TriggerResultLabel", edm::InputTag("TriggerResults", "", "HLT"));
  desc.add<edm::InputTag>("MuonLabel", edm::InputTag("muons"));
  desc.add<double>("MinimumMuonP", 10.0);
  desc.add<std::vector<std::string> >("Triggers", triggers);
  desc.add<bool>("PFCut", true);
  desc.add<double>("PFIsolationCut", 0.12);
  desc.add<double>("TrackIsolationCut", 3.0);
  desc.add<double>("CaloIsolationCut", 5.0);
  desc.add<int>("PreScale", 2);
  descriptions.add("AlcaHBHEMuonFilter", desc);
}

//define this as a plug-in
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(AlCaHBHEMuonFilter);
