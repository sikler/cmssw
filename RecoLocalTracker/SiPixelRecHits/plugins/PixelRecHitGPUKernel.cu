// C++ headers
#include <algorithm>
#include <numeric>

// CUDA runtime
#include <cuda_runtime.h>

// CMSSW headers
#include "CUDADataFormats/SiPixelCluster/interface/gpuClusteringConstants.h"
#include "HeterogeneousCore/CUDAUtilities/interface/cudaCheck.h"
#include "HeterogeneousCore/CUDAUtilities/interface/device_unique_ptr.h"

#include "PixelRecHitGPUKernel.h"
#include "gpuPixelRecHits.h"

namespace {
  __global__ void setHitsLayerStart(uint32_t const* __restrict__ hitsModuleStart,
                                    pixelCPEforGPU::ParamsOnGPU const* cpeParams,
                                    uint32_t* hitsLayerStart) {
    auto i = blockIdx.x * blockDim.x + threadIdx.x;

    assert(0 == hitsModuleStart[0]);

    if (i < 11) {
      hitsLayerStart[i] = hitsModuleStart[cpeParams->layerGeometry().layerStart[i]];
#ifdef GPU_DEBUG
      printf("LayerStart %d %d: %d\n", i, cpeParams->layerGeometry().layerStart[i], hitsLayerStart[i]);
#endif
    }
  }
}  // namespace

namespace pixelgpudetails {

  TrackingRecHit2DGPU PixelRecHitGPUKernel::makeHitsAsync(SiPixelDigisCUDA const& digis_d,
                                                          SiPixelClustersCUDA const& clusters_d,
                                                          BeamSpotCUDA const& bs_d,
                                                          pixelCPEforGPU::ParamsOnGPU const* cpeParams,
                                                          cudaStream_t stream) const {
    auto nHits = clusters_d.nClusters();
    TrackingRecHit2DGPU hits_d(nHits, cpeParams, clusters_d.clusModuleStart(), stream);

    int threadsPerBlock = 128;
    int blocks = digis_d.nModules();  // active modules (with digis)

#ifdef GPU_DEBUG
    std::cout << "launching getHits kernel for " << blocks << " blocks" << std::endl;
#endif
    // protect from empty events
    if (blocks) {
      gpuPixelRecHits::getHits<<<blocks, threadsPerBlock, 0, stream>>>(
          cpeParams, bs_d.data(), digis_d.view(), digis_d.nDigis(), clusters_d.view(), hits_d.view());
      cudaCheck(cudaGetLastError());
#ifdef GPU_DEBUG
      cudaCheck(cudaDeviceSynchronize());
#endif
    }

    // assuming full warp of threads is better than a smaller number...
    if (nHits) {
      setHitsLayerStart<<<1, 32, 0, stream>>>(clusters_d.clusModuleStart(), cpeParams, hits_d.hitsLayerStart());
      cudaCheck(cudaGetLastError());

      cms::cuda::fillManyFromVector(
          hits_d.phiBinner(), 10, hits_d.iphi(), hits_d.hitsLayerStart(), nHits, 256, hits_d.phiBinnerStorage(), stream);
      cudaCheck(cudaGetLastError());

#ifdef GPU_DEBUG
      cudaCheck(cudaDeviceSynchronize());
#endif
    }

    return hits_d;
  }

}  // namespace pixelgpudetails
