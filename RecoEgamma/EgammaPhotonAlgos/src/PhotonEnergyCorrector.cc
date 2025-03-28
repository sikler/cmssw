#include "RecoEgamma/EgammaPhotonAlgos/interface/PhotonEnergyCorrector.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterFunctionFactory.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloTopology/interface/CaloTopology.h"
#include "Geometry/Records/interface/CaloTopologyRecord.h"
#include "Geometry/CaloTopology/interface/CaloTopology.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"

#include "RecoEgamma/EgammaPhotonAlgos/interface/EnergyUncertaintyPhotonSpecific.h"

PhotonEnergyCorrector::PhotonEnergyCorrector(const edm::ParameterSet& config, edm::ConsumesCollector&& iC)
    : ecalClusterToolsESGetTokens_{iC}, caloGeomToken_{iC.esConsumes()} {
  minR9Barrel_ = config.getParameter<double>("minR9Barrel");
  minR9Endcap_ = config.getParameter<double>("minR9Endcap");
  // get the geometry from the event setup:

  barrelEcalHits_ = config.getParameter<edm::InputTag>("barrelEcalHits");
  endcapEcalHits_ = config.getParameter<edm::InputTag>("endcapEcalHits");
  barrelEcalHitsToken_ = iC.consumes<EcalRecHitCollection>(config.getParameter<edm::InputTag>("barrelEcalHits"));
  endcapEcalHitsToken_ = iC.consumes<EcalRecHitCollection>(config.getParameter<edm::InputTag>("endcapEcalHits"));

  //  candidateP4type_ = config.getParameter<std::string>("candidateP4type") ;

  // function to extract f(eta) correction
  std::string superClusterFunctionName = config.getParameter<std::string>("superClusterEnergyCorrFunction");
  scEnergyFunction_ = EcalClusterFunctionFactory::get()->create(superClusterFunctionName, config, iC);

  // function to extract corrections to cracks
  std::string superClusterCrackFunctionName = config.getParameter<std::string>("superClusterCrackEnergyCorrFunction");
  scCrackEnergyFunction_ = EcalClusterFunctionFactory::get()->create(superClusterCrackFunctionName, config, iC);

  // function to extract the error on the sc ecal correction
  std::string superClusterErrorFunctionName = config.getParameter<std::string>("superClusterEnergyErrorFunction");
  scEnergyErrorFunction_ = EcalClusterFunctionFactory::get()->create(superClusterErrorFunctionName, config, iC);

  // function  to extract the error on the photon ecal correction
  std::string photonEnergyFunctionName = config.getParameter<std::string>("photonEcalEnergyCorrFunction");
  photonEcalEnergyCorrFunction_ = EcalClusterFunctionFactory::get()->create(photonEnergyFunctionName, config, iC);
  //ingredient for photon uncertainty
  photonUncertaintyCalculator_ = std::make_unique<EnergyUncertaintyPhotonSpecific>(config);

  if (config.existsAs<edm::ParameterSet>("regressionConfig")) {
    const edm::ParameterSet& regr_conf = config.getParameterSet("regressionConfig");
    const std::string& mname = regr_conf.getParameter<std::string>("modifierName");
    gedRegression_ = ModifyObjectValueFactory::get()->create(mname, regr_conf, iC);
  }

  // ingredient for energy regression
  weightsfromDB_ = config.getParameter<bool>("regressionWeightsFromDB");
  w_file_ = config.getParameter<std::string>("energyRegressionWeightsFileLocation");
  if (weightsfromDB_)
    w_db_ = config.getParameter<std::string>("energyRegressionWeightsDBLocation");
  else
    w_db_ = "none";
  regressionCorrector_ = std::make_unique<EGEnergyCorrector>();
}

void PhotonEnergyCorrector::init(const edm::EventSetup& theEventSetup) {
  theCaloGeom_ = theEventSetup.getHandle(caloGeomToken_);

  scEnergyFunction_->init(theEventSetup);
  scCrackEnergyFunction_->init(theEventSetup);
  scEnergyErrorFunction_->init(theEventSetup);
  photonEcalEnergyCorrFunction_->init(theEventSetup);

  if (weightsfromDB_) {
    if (!regressionCorrector_->IsInitialized())
      regressionCorrector_->Initialize(theEventSetup, w_db_, weightsfromDB_);
  }
  if (!weightsfromDB_ && !(w_file_ == "none")) {
    if (!regressionCorrector_->IsInitialized())
      regressionCorrector_->Initialize(theEventSetup, w_file_, weightsfromDB_);
  }

  photonUncertaintyCalculator_->init(theEventSetup);
}

void PhotonEnergyCorrector::calculate(edm::Event& evt,
                                      reco::Photon& thePhoton,
                                      int subdet,
                                      const reco::VertexCollection& vtxcol,
                                      const edm::EventSetup& iSetup) {
  double phoEcalEnergy = -9999.;
  double phoEcalEnergyError = -9999.;
  double phoRegr1Energy = -9999.;
  double phoRegr1EnergyError = -9999.;
  theCaloGeom_->getSubdetectorGeometry(DetId::Ecal, subdet);

  double minR9 = 0;
  if (subdet == EcalBarrel) {
    minR9 = minR9Barrel_;
  } else if (subdet == EcalEndcap) {
    minR9 = minR9Endcap_;
  }

  EcalClusterLazyTools lazyTools(
      evt, ecalClusterToolsESGetTokens_.get(iSetup), barrelEcalHitsToken_, endcapEcalHitsToken_);

  ////////////// Here default Ecal corrections based on electrons  ////////////////////////
  if (thePhoton.r9() > minR9) {
    // f(eta) correction to e5x5
    double deltaE = scEnergyFunction_->getValue(*(thePhoton.superCluster()), 1);
    float e5x5 = thePhoton.e5x5();
    if (subdet == EcalBarrel)
      e5x5 = e5x5 * (1.0 + deltaE / thePhoton.superCluster()->rawEnergy());
    phoEcalEnergy = e5x5 + thePhoton.superCluster()->preshowerEnergy();
  } else {
    phoEcalEnergy = thePhoton.superCluster()->energy();
  }
  // store the value in the Photon.h
  thePhoton.setCorrectedEnergy(reco::Photon::ecal_standard, phoEcalEnergy, phoEcalEnergyError, false);

  ////////////// Here Ecal corrections specific for photons ////////////////////////

  if (thePhoton.r9() > minR9) {
    // f(eta) correction to e5x5
    double deltaE = scEnergyFunction_->getValue(*(thePhoton.superCluster()), 1);
    float e5x5 = thePhoton.e5x5();
    if (subdet == EcalBarrel)
      e5x5 = e5x5 * (1.0 + deltaE / thePhoton.superCluster()->rawEnergy());
    phoEcalEnergy = e5x5 + thePhoton.superCluster()->preshowerEnergy();
    // add correction for cracks
    phoEcalEnergy *= scCrackEnergyFunction_->getValue(*(thePhoton.superCluster()));
    phoEcalEnergyError = photonUncertaintyCalculator_->computePhotonEnergyUncertainty_highR9(
        thePhoton.superCluster()->eta(),
        thePhoton.superCluster()->phiWidth() / thePhoton.superCluster()->etaWidth(),
        phoEcalEnergy);
  } else {
    // correction for low r9
    phoEcalEnergy = photonEcalEnergyCorrFunction_->getValue(*(thePhoton.superCluster()), 1);
    phoEcalEnergy *= applyCrackCorrection(*(thePhoton.superCluster()), scCrackEnergyFunction_.get());
    phoEcalEnergyError = photonUncertaintyCalculator_->computePhotonEnergyUncertainty_lowR9(
        thePhoton.superCluster()->eta(),
        thePhoton.superCluster()->phiWidth() / thePhoton.superCluster()->etaWidth(),
        phoEcalEnergy);
  }

  // store the value in the Photon.h
  thePhoton.setCorrectedEnergy(reco::Photon::ecal_photons, phoEcalEnergy, phoEcalEnergyError, false);

  //////////  Energy  Regression //////////////////////
  //
  if ((weightsfromDB_ && !gedRegression_) || (!weightsfromDB_ && !(w_file_ == "none"))) {
    std::pair<double, double> cor =
        regressionCorrector_->CorrectedEnergyWithError(thePhoton, vtxcol, lazyTools, *theCaloGeom_);
    phoRegr1Energy = cor.first;
    phoRegr1EnergyError = cor.second;
    // store the value in the Photon.h
    thePhoton.setCorrectedEnergy(reco::Photon::regression1, phoRegr1Energy, phoRegr1EnergyError, false);
  }

  if (gedRegression_) {
    gedRegression_->modifyObject(thePhoton);  // uses regression2 slot
    // force regresions1 and 2 to be the same (no reason to be different)
    thePhoton.setCorrectedEnergy(reco::Photon::regression1,
                                 thePhoton.getCorrectedEnergy(reco::Photon::regression2),
                                 thePhoton.getCorrectedEnergyError(reco::Photon::regression2),
                                 false);
  }

  /*
  std::cout << " ------------------------- " << std::endl;
  std::cout << " Corrector " << std::endl;
  std::cout << " P4 Type " << thePhoton.getCandidateP4type() << " candidate p4 " << thePhoton.p4() << std::endl;
  std::cout << " photon ecalEnergy " << thePhoton.getCorrectedEnergy(reco::Photon::ecal_photons) << " error " << thePhoton.getCorrectedEnergyError(reco::Photon::ecal_photons) << std::endl;
  std::cout << " ecal p4 from accessor " << thePhoton.p4(reco::Photon::ecal_photons) <<  std::endl;
  std::cout << " ------------------------- " << std::endl;
  std::cout << " reg1 energy " << thePhoton.getCorrectedEnergy(reco::Photon::regression1)  << " error " <<  thePhoton.getCorrectedEnergyError(reco::Photon::regression1) << std::endl;
  std::cout << " New p4 from regression " <<  thePhoton.p4(reco::Photon::regression1)    << std::endl;
  std::cout << " ------------------------- " << std::endl;
  */
}

double PhotonEnergyCorrector::applyCrackCorrection(const reco::SuperCluster& cl,
                                                   EcalClusterFunctionBaseClass* crackCorrectionFunction) {
  double crackcor = 1.;

  for (reco::CaloCluster_iterator cIt = cl.clustersBegin(); cIt != cl.clustersEnd(); ++cIt) {
    const reco::CaloClusterPtr cc = *cIt;
    crackcor *= ((cl.rawEnergy() + cc->energy() * (crackCorrectionFunction->getValue(*cc) - 1.)) / cl.rawEnergy());
  }  // loop on BCs

  return crackcor;
}
