import FWCore.ParameterSet.Config as cms
from Configuration.Eras.Modifier_h2tb_cff import h2tb

process = cms.Process("PROD", h2tb)

process.load("SimGeneral.HepPDTESSource.pdt_cfi")
process.load("SimG4CMS.HcalTestBeam.TB2003GeometryXML_cfi")
process.load("Geometry.HcalTestBeamData.hcalDDDSimConstants_cff")
process.load("Geometry.EcalCommonData.ecalSimulationParameters_cff")
process.load('GeneratorInterface.Core.generatorSmeared_cfi')
process.load("Configuration.EventContent.EventContent_cff")
process.load("SimG4Core.Application.g4SimHits_cfi")

process.TFileService = cms.Service("TFileService",
    fileName = cms.string('hcaltb03.root')
)

process.load('FWCore.MessageService.MessageLogger_cfi')
if hasattr(process,'MessageLogger'):
    process.MessageLogger.HcalTBSim=dict()

process.load("IOMC.RandomEngine.IOMC_cff")
process.RandomNumberGeneratorService.generator.initialSeed = 456789
process.RandomNumberGeneratorService.g4SimHits.initialSeed = 9876
process.RandomNumberGeneratorService.VtxSmeared.initialSeed = 123456789

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100)
)

beamPosition = -408.50
process.common_beam_direction_parameters = cms.PSet(
    MinEta       = cms.double(0.5655),
    MaxEta       = cms.double(0.5655),
    MinPhi       = cms.double(0.1309),
    MaxPhi       = cms.double(0.1309),
    BeamPosition = cms.double(beamPosition)
)

from IOMC.EventVertexGenerators.VtxSmearedParameters_cfi import *
process.VtxSmeared = cms.EDProducer("BeamProfileVtxGenerator",
    process.common_beam_direction_parameters,
    VtxSmearedCommon,
    BeamMeanX       = cms.double(0.0),
    BeamMeanY       = cms.double(0.0),
    BeamSigmaX      = cms.double(0.0001),
    BeamSigmaY      = cms.double(0.0001),
    Psi             = cms.double(999.9),
    GaussianProfile = cms.bool(False),
    BinX            = cms.int32(50),
    BinY            = cms.int32(50),
    File            = cms.string('beam.profile'),
    UseFile         = cms.bool(False),
    TimeOffset      = cms.double(0.)
)

process.source = cms.Source("EmptySource",
    firstRun        = cms.untracked.uint32(1),
    firstEvent      = cms.untracked.uint32(1)
)

process.generator = cms.EDProducer("FlatRandomEGunProducer",
    PGunParameters = cms.PSet(
        process.common_beam_direction_parameters,
        MinE   = cms.double(9.99),
        MaxE   = cms.double(10.01),
        PartID = cms.vint32(211)
    ),
    Verbosity       = cms.untracked.int32(0),
    AddAntiParticle = cms.bool(False)
)

process.o1 = cms.OutputModule("PoolOutputModule",
    process.FEVTSIMEventContent,
    fileName = cms.untracked.string('sim2003.root')
)

process.Timing = cms.Service("Timing")

process.p1 = cms.Path(process.generator*process.VtxSmeared*process.generatorSmeared*process.g4SimHits)
process.outpath = cms.EndPath(process.o1)
process.g4SimHits.NonBeamEvent = True
process.g4SimHits.UseMagneticField = False
process.g4SimHits.Physics.type = 'SimG4Core/Physics/QGSP_FTFP_BERT_EML'
process.g4SimHits.Physics.Region = 'HcalRegion'
process.g4SimHits.Physics.DefaultCutValue = 1.

process.g4SimHits.StackingAction.KillGamma = False
process.g4SimHits.CaloSD.BeamPosition = beamPosition
process.g4SimHits.ECalSD.UseBirkLaw = True
process.g4SimHits.ECalSD.BirkL3Parametrization = True
process.g4SimHits.ECalSD.BirkC1 = 0.033
process.g4SimHits.ECalSD.BirkC2 = 0.0
process.g4SimHits.ECalSD.SlopeLightYield = 0.02
process.g4SimHits.HCalSD.UseBirkLaw = True
process.g4SimHits.HCalSD.BirkC1 = 0.0052
process.g4SimHits.HCalSD.BirkC2 = 0.142
process.g4SimHits.HCalSD.BirkC3 = 1.75
process.g4SimHits.HCalSD.UseLayerWt = False
process.g4SimHits.HCalSD.WtFile     = ' '
process.g4SimHits.HCalSD.UseShowerLibrary    = False
process.g4SimHits.HCalSD.TestNumberingScheme = False
process.g4SimHits.HCalSD.UseHF   = False
process.g4SimHits.HCalSD.ForTBHCAL = True
process.g4SimHits.HCalSD.ForTBH2 = True
process.g4SimHits.OnlySDs = ['CaloTrkProcessing',
                             'EcalSensitiveDetector',
                             'HcalSensitiveDetector']
process.g4SimHits.CaloTrkProcessing.TestBeam = True

#process.g4SimHits.Watchers = cms.VPSet(cms.PSet(
#    HcalTB04Analysis = cms.PSet(
#        process.common_beam_direction_parameters,
#        HcalOnly  = cms.bool(False),
#        Type      = cms.int32(2),
#        Mode      = cms.int32(1),
#        ScaleHB0  = cms.double(0.5),
#        ScaleHB16 = cms.double(0.5),
#        ScaleHE0  = cms.double(0.5),
#        ScaleHO   = cms.double(0.4),
#        EcalNoise = cms.double(0.13),
#        Names     = cms.vstring('HcalHits', 'EcalHitsEB'),
#        Verbose   = cms.untracked.bool(True),
#        FileName  = cms.untracked.string('HcalTB03.root'),
#        ETtotMax  = cms.untracked.double(20.0),
#        EHCalMax  = cms.untracked.double(2.0)
#    ),
#    HcalQie = cms.PSet(
#        NumOfBuckets  = cms.int32(10),
#        BaseLine      = cms.int32(4),
#        BinOfMax      = cms.int32(6),
#        PreSamples    = cms.int32(0),
#        EDepPerPE     = cms.double(0.0005),
#        SignalBuckets = cms.int32(2),
#        SigmaNoise    = cms.double(0.5),
#        qToPE         = cms.double(4.0)
#    ),
#    type = cms.string('HcalTB04Analysis')
#))

