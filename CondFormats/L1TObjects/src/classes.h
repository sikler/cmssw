#include "CondFormats/L1TObjects/interface/L1MuScale.h"
#include "CondFormats/L1TObjects/interface/L1MuTriggerScales.h"
#include "CondFormats/L1TObjects/interface/L1MuTriggerPtScale.h"
#include "CondFormats/L1TObjects/interface/L1MuGMTParameters.h"
#include "CondFormats/L1TObjects/interface/L1MuGMTChannelMask.h"
#include "CondFormats/L1TObjects/interface/L1MuGMTScales.h"
#include "CondFormats/L1TObjects/interface/L1CaloEtScale.h"
#include "CondFormats/L1TObjects/interface/L1GctChannelMask.h"
#include "CondFormats/L1TObjects/interface/L1GctJetFinderParams.h"
#include "CondFormats/L1TObjects/interface/L1MuDTExtLut.h"
#include "CondFormats/L1TObjects/interface/L1MuDTPhiLut.h"
#include "CondFormats/L1TObjects/interface/L1MuDTPtaLut.h"
#include "CondFormats/L1TObjects/interface/L1MuDTEtaPatternLut.h"
#include "CondFormats/L1TObjects/interface/L1MuDTQualPatternLut.h"
#include "CondFormats/L1TObjects/interface/L1MuDTTFParameters.h"
#include "CondFormats/L1TObjects/interface/L1MuDTTFMasks.h"
#include "CondFormats/L1TObjects/interface/L1MuCSCTFConfiguration.h"
#include "CondFormats/L1TObjects/interface/L1MuCSCPtLut.h"
#include "CondFormats/L1TObjects/interface/L1MuCSCTFAlignment.h"
#include "CondFormats/L1TObjects/interface/L1RPCConfig.h"
#include "CondFormats/L1TObjects/interface/L1RPCConeDefinition.h"
#include "CondFormats/L1TObjects/interface/L1RCTParameters.h"
#include "CondFormats/L1TObjects/interface/L1RCTChannelMask.h"
#include "CondFormats/L1TObjects/interface/L1RCTNoisyChannelMask.h"
#include "CondFormats/L1TObjects/interface/L1CaloEcalScale.h"
#include "CondFormats/L1TObjects/interface/L1CaloHcalScale.h"
#include "CondFormats/L1TObjects/interface/L1TriggerKey.h"
#include "CondFormats/L1TObjects/interface/L1TriggerKeyList.h"
#include "CondFormats/L1TObjects/interface/L1TriggerKeyExt.h"
#include "CondFormats/L1TObjects/interface/L1TriggerKeyListExt.h"

#include "CondFormats/L1TObjects/interface/L1GtStableParameters.h"
#include "CondFormats/L1TObjects/interface/L1GtParameters.h"
#include "CondFormats/L1TObjects/interface/L1GtPrescaleFactors.h"
#include "CondFormats/L1TObjects/interface/L1GtTriggerMask.h"
#include "CondFormats/L1TObjects/interface/L1GtBoard.h"
#include "CondFormats/L1TObjects/interface/L1GtBoardMaps.h"
#include "CondFormats/L1TObjects/interface/L1GtPsbConfig.h"
#include "CondFormats/L1TObjects/interface/L1GtPsbSetup.h"
#include "CondFormats/L1TObjects/interface/L1GtCondition.h"
#include "CondFormats/L1TObjects/interface/L1GtAlgorithm.h"
#include "CondFormats/L1TObjects/interface/L1GtTriggerMenu.h"
#include "CondFormats/L1TObjects/interface/GlobalStableParameters.h"

#include "CondFormats/L1TObjects/interface/L1CaloGeometry.h"
#include "CondFormats/L1TObjects/interface/L1RPCBxOrConfig.h"
#include "CondFormats/L1TObjects/interface/L1RPCHsbConfig.h"

#include "CondFormats/L1TObjects/interface/LUT.h"
#include "CondFormats/L1TObjects/interface/CaloParams.h"
#include "CondFormats/L1TObjects/interface/CaloConfig.h"

#include "CondFormats/L1TObjects/interface/L1TMuonOverlapParams.h"
#include "CondFormats/L1TObjects/interface/L1TMuonGlobalParams.h"
#include "CondFormats/L1TObjects/interface/L1TMuonBarrelParams.h"
#include "CondFormats/L1TObjects/interface/L1TMuonBarrelKalmanParams.h"
#include "CondFormats/L1TObjects/interface/L1TMuonEndCapParams.h"
#include "CondFormats/L1TObjects/interface/L1TMuonEndCapForest.h"

#include "CondFormats/L1TObjects/interface/L1TGlobalParameters.h"
#include "CondFormats/L1TObjects/interface/L1TUtmAlgorithm.h"
#include "CondFormats/L1TObjects/interface/L1TUtmBin.h"
#include "CondFormats/L1TObjects/interface/L1TUtmCondition.h"
#include "CondFormats/L1TObjects/interface/L1TUtmCut.h"
#include "CondFormats/L1TObjects/interface/L1TUtmCutValue.h"
#include "CondFormats/L1TObjects/interface/L1TUtmObject.h"
#include "CondFormats/L1TObjects/interface/L1TUtmScale.h"
#include "CondFormats/L1TObjects/interface/L1TUtmTriggerMenu.h"

#include "CondFormats/L1TObjects/interface/L1TGlobalPrescalesVetos.h"
#include "CondFormats/L1TObjects/interface/L1TGlobalPrescalesVetosFract.h"

namespace CondFormats_L1TObjects {
  struct dictionary {
    std::vector<l1t::CaloParams::Node> dummy1a;
    l1t::CaloParams dummy1b;
    l1t::LUT dummy1c;

    l1t::CaloConfig dummy2;

    std::vector<L1MuDTExtLut::LUT> dummy3;
    std::vector<L1GtMuonTemplate> dummy4;
    std::vector<L1GtCaloTemplate> dummy5;
    std::vector<L1GtEnergySumTemplate> dummy6;
    std::vector<L1GtJetCountsTemplate> dummy7;
    std::vector<L1GtCorrelationTemplate> dummy8;
    std::vector<L1GtCastorTemplate> dummy8a;
    std::vector<L1GtHfBitCountsTemplate> dummy8b;
    std::vector<L1GtHfRingEtSumsTemplate> dummy8c;
    std::vector<L1GtBptxTemplate> dummy8d;
    std::vector<L1GtExternalTemplate> dummy8e;
    std::map<std::string, L1GtAlgorithm> dummy9;
    std::pair<std::string, L1GtAlgorithm> dummy9a;
    std::pair<short, L1MuDTEtaPattern> dummy11;
    std::pair<int, std::vector<L1GtObject> > dummy13;
    //    std::vector<L1RPCConeDefinition::TLPSize> dummy15;
    //    std::vector<L1RPCConeDefinition::TRingToTower> dummy15a;
    //    std::vector<L1RPCConeDefinition::TRingToLP> dummy15b;
    L1TMuonGlobalParams dummy16;
    std::vector<L1TMuonGlobalParams::Node> dummy16a;

    L1TMuonOverlapParams dummy17;
    std::vector<L1TMuonOverlapParams::Node> dummy17a;

    L1TMuonBarrelParams dummy18;
    std::vector<L1TMuonBarrelParams::Node> dummy18a;
    L1TMuonBarrelKalmanParams dummy18b;
    std::vector<L1TMuonBarrelKalmanParams::Node> dummy18c;

    L1TMuonEndCapParams dummy19;

    L1TMuonEndCapForest dummy20a;
    L1TMuonEndCapForest::DTree dummy20b;
    L1TMuonEndCapForest::DForest dummy20c;
    L1TMuonEndCapForest::DForestColl dummy20d;
    L1TMuonEndCapForest::DForestMap dummy20e;

    L1TUtmAlgorithm dummy21a;
    L1TUtmBin dummy21b;
    L1TUtmCondition dummy21c;
    L1TUtmCut dummy21d;
    L1TUtmCutValue dummy21e;
    L1TUtmObject dummy21f;
    L1TUtmScale dummy21g;
    L1TUtmTriggerMenu dummy21h;

    L1TGlobalPrescalesVetos dummy22;
    L1TGlobalPrescalesVetosFract dummy22a;

    L1TGlobalParameters dummy23;
  };
}  // namespace CondFormats_L1TObjects
