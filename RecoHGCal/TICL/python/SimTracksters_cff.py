import FWCore.ParameterSet.Config as cms

from RecoHGCal.TICL.trackstersFromSimClustersProducer_cfi import trackstersFromSimClustersProducer as _trackstersFromSimClustersProducer
from RecoHGCal.TICL.trackstersFromCaloParticlesProducer_cfi import trackstersFromCaloParticlesProducer as _trackstersFromCaloParticlesProducer
from RecoHGCal.TICL.filteredLayerClustersProducer_cfi import filteredLayerClustersProducer as _filteredLayerClustersProducer


# CA - PATTERN RECOGNITION


filteredLayerClustersSimTracksters = _filteredLayerClustersProducer.clone(
    clusterFilter = "ClusterFilterByAlgoAndSize",
    algo_number = 8,
    min_cluster_size = 0, # inclusive
    iteration_label = "ticlSimTracksters"
)

ticlSimTracksters = _trackstersFromSimClustersProducer.clone(
)
ticlSimTrackstersFromCP = _trackstersFromCaloParticlesProducer.clone(
)

from Configuration.ProcessModifiers.premix_stage2_cff import premix_stage2
premix_stage2.toModify(ticlSimTracksters,
    simclusters = "mixData:MergedCaloTruth",
    caloparticles = "mixData:MergedCaloTruth",
)
premix_stage2.toModify(ticlSimTrackstersFromCP,
    caloparticles = "mixData:MergedCaloTruth",
)

ticlSimTrackstersTask = cms.Task(filteredLayerClustersSimTracksters, ticlSimTracksters, ticlSimTrackstersFromCP)
