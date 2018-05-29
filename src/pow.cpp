// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>
#include <chain.h>
#include <chainparams.h>
#include <arith_uint256.h>
#include <chain.h>
#include <validation.h>
#include <primitives/block.h>
#include <uint256.h>


extern CBlockIndex *pindexBestHeader;
extern CChain chainActive;
extern BlockMap& mapBlockIndex;


const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake, const Consensus::Params& params)
{
    while (pindex && pindex->pprev && (pindex->IsProofOfStake() != fProofOfStake))
    {
    	if(fProofOfStake)
    	{
    		if(pindex->nHeight <=params.UBCONTRACT_Height)
				return NULL;
    	}
        pindex = pindex->pprev;
    }
    return pindex;
}


unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock,const Consensus::Params& params)
{
    assert(pindexLast != nullptr);
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    if ((pindexLast->nHeight+1)== Params().GetConsensus().UBCHeight)  
    {
        const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
        arith_uint256 bnNew;
        bnNew.SetCompact(pindexLast->nBits);
        bnNew *= 100;
        if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;
        
        return bnNew.GetCompact();
    }
    else if  ((pindexLast->nHeight+1)== Params().GetConsensus().ForkV1Height)  
    {
        const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
        arith_uint256 bnNew;
        bnNew.SetCompact(pindexLast->nBits);
        bnNew *= 100;
        if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;
        
        return bnNew.GetCompact();
    }
	
    Consensus::Params * temp_params = (Consensus::Params *)&params;

    if ((pindexLast->nHeight+1) >= Params().GetConsensus().UBCONTRACT_Height)
    {
    	temp_params->UpdateDifficultyAdjustmentIntervalForkV2();
    }
    else if ((pindexLast->nHeight+1) >= Params().GetConsensus().ForkV1Height)
    {
    	temp_params->UpdateDifficultyAdjustmentIntervalForkV1();
    }
   else if((pindexLast->nHeight+1) >= Params().GetConsensus().UBCHeight + Params().GetConsensus().UBCInitBlockCount)
    {
		temp_params->UpdateDifficultyAdjustmentInterval();
    }
    else
    {
	temp_params->UpdateOldDifficultyAdjustmentInterval();
    }
    
    if(!params.is_regtest_net) 
    {
        if ((pindexLast->nHeight + 1) >=Params().GetConsensus().UBCHeight + Params().GetConsensus().UBCInitBlockCount) 
        {
            temp_params->UpdateDifficultyAdjustmentInterval();
        } 
        else 
        {
            temp_params->UpdateOldDifficultyAdjustmentInterval();
        }
    }
    // Only change once per difficulty adjustment interval
    if ((((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)) &&((pindexLast->nHeight+1) < params.UBCONTRACT_Height) )
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                {
                    if(pindex->IsProofOfStake())
                    {
                        pindex = pindex->pprev;
                        continue;
                    }
                    pindex = pindex->pprev;
                }
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }
	
	if(pblock->IsProofOfStake() && (pindexLast->nHeight+1 >=params.UBCONTRACT_Height))
	{
	  return PosGetNextTargetRequired(pindexLast,pblock,params);
	}

	if(pblock->IsProofOfWork() && (pindexLast->nHeight+1 >=params.UBCONTRACT_Height))
	{
	  return PowGetNextTargetRequired(pindexLast,pblock,params);
	}

    // Go back by what we want to be 14 days worth of blocks
    int nHeightFirst = pindexLast->nHeight - (params.DifficultyAdjustmentInterval()-1);
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

unsigned int PosGetNextTargetRequired(const CBlockIndex* pindexLast,  const CBlockHeader *pblock, const Consensus::Params& params)
{
	bool fProofOfStake = pblock->IsProofOfStake();
	uint32_t nLastPosBits = 0;
	int64_t nLastPosTime = 0;
	assert(fProofOfStake == true);
    unsigned int bnTargetLimitnBits = UintToArith256(params.posLimit).GetCompact() ;

    if (pindexLast == NULL )
        //return bnTargetLimit.GetCompact(); // genesis block
        return bnTargetLimitnBits;

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake,params);//获取上一个POS块的index
	if(pindexPrev == NULL)
		return bnTargetLimitnBits;
	else
	{
	    nLastPosBits = pindexPrev->nBits;
	    nLastPosTime = pindexPrev->GetBlockTime();
	}
	    
    if (pindexPrev->pprev == NULL)
        //return bnTargetLimit.GetCompact(); // first block
        return bnTargetLimitnBits;
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake,params);
	if(pindexPrevPrev == NULL)
		return bnTargetLimitnBits;
    if (pindexPrevPrev->pprev == NULL)
        //return bnTargetLimit.GetCompact(); // second block
        return bnTargetLimitnBits;

    std::vector<unsigned int> tNbits;
    int64_t nFirstBlockTime=0;
    const CBlockIndex* tIndexLast = pindexLast;
    while (tIndexLast && tIndexLast->pprev)
    {
        if(tNbits.size() == (params.nPowTargetTimespan / params.nPowTargetSpacing) || tIndexLast->nHeight <=params.UBCONTRACT_Height)
            break;
		if(tIndexLast->IsProofOfStake())
		{
		    nFirstBlockTime = tIndexLast->GetBlockTime();
		    tNbits.push_back(tIndexLast->nBits);
        }
        tIndexLast = tIndexLast->pprev;
    }

    if(tNbits.size() < (params.nPowTargetTimespan / params.nPowTargetSpacing))
        return bnTargetLimitnBits;
    
    std::set<unsigned int> tmpBits;

    for(int i = 0;i < tNbits.size();i++)
    {
        tmpBits.insert(tNbits[i]);
    }

    if(tmpBits.size() == 1)
    {
        // Limit adjustment step
        int64_t nActualTimespan = nLastPosTime - nFirstBlockTime;
        if (nActualTimespan < params.nPowTargetTimespan/4)
            nActualTimespan = params.nPowTargetTimespan/4;
        if (nActualTimespan > params.nPowTargetTimespan*4)
            nActualTimespan = params.nPowTargetTimespan*4;
    
        // Retarget
        const arith_uint256 bnPosLimit = UintToArith256(params.posLimit);
        arith_uint256 bnNew;
        bnNew.SetCompact(nLastPosBits);
        bnNew *= nActualTimespan;
        bnNew /= params.nPowTargetTimespan;
    
        if (bnNew > bnPosLimit)
            bnNew = bnPosLimit;
    
        return bnNew.GetCompact();
    }
    else
    {
        return nLastPosBits;
    }
}

unsigned int PowGetNextTargetRequired(const CBlockIndex* pindexLast,  const CBlockHeader *pblock, const Consensus::Params& params)
{
	bool fProofOfWork = pblock->IsProofOfWork();
	uint32_t nLastPowBits = 0;
	int64_t nLastPowTime = 0;
	assert(fProofOfWork == true);
    unsigned int bnTargetLimitnBits = UintToArith256(params.powLimit).GetCompact() ;

    if (pindexLast == NULL )
        //return bnTargetLimit.GetCompact(); // genesis block
        return bnTargetLimitnBits;

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, false,params);//获取上一个POS块的index
	if(pindexPrev == NULL)
		return bnTargetLimitnBits;
	else
	{
	    nLastPowBits = pindexPrev->nBits;
	    nLastPowTime = pindexPrev->GetBlockTime();
	}
	    
    if (pindexPrev->pprev == NULL)
        //return bnTargetLimit.GetCompact(); // first block
        return bnTargetLimitnBits;
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, false,params);
	if(pindexPrevPrev == NULL)
		return bnTargetLimitnBits;
    if (pindexPrevPrev->pprev == NULL)
        //return bnTargetLimit.GetCompact(); // second block
        return bnTargetLimitnBits;

    std::vector<unsigned int> tNbits;
    int64_t nFirstBlockTime=0;
    const CBlockIndex* tIndexLast = pindexLast;
    while (tIndexLast && tIndexLast->pprev)
    {
        if(tNbits.size() == (params.nPowTargetTimespan / params.nPowTargetSpacing))
            break;
			if(tIndexLast->IsProofOfWork())
			{
			    nFirstBlockTime = tIndexLast->GetBlockTime();
			    tNbits.push_back(tIndexLast->nBits);
	    }
        tIndexLast = tIndexLast->pprev;
    }

    if(tNbits.size() < (params.nPowTargetTimespan / params.nPowTargetSpacing))
        return bnTargetLimitnBits;
    
    std::set<unsigned int> tmpBits;

    for(int i = 0;i < tNbits.size();i++)
    {
        tmpBits.insert(tNbits[i]);
    }

    if(tmpBits.size() == 1)
    {
        // Limit adjustment step
        int64_t nActualTimespan = nLastPowTime - nFirstBlockTime;
        if (nActualTimespan < params.nPowTargetTimespan/4)
            nActualTimespan = params.nPowTargetTimespan/4;
        if (nActualTimespan > params.nPowTargetTimespan*4)
            nActualTimespan = params.nPowTargetTimespan*4;
    
        // Retarget
        const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
        arith_uint256 bnNew;
        bnNew.SetCompact(nLastPowBits);
        bnNew *= nActualTimespan;
        bnNew /= params.nPowTargetTimespan;
    
        if (bnNew > bnPowLimit)
            bnNew = bnPowLimit;
    
        return bnNew.GetCompact();
    }
    else
    {
        return nLastPowBits;
    }
}



unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, uint256 prevHash, unsigned int nBits, const Consensus::Params& params, int blockHeight)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

	if (blockHeight != -1) {
		if ((blockHeight >= Params().GetConsensus().UBCHeight) 
			&& (blockHeight < (Params().GetConsensus().UBCHeight + Params().GetConsensus().UBCInitBlockCount)))
			return true;
	}

	auto iter = mapBlockIndex.find(prevHash);
	if (iter != mapBlockIndex.end()) {
		if ((iter->second->nHeight >= Params().GetConsensus().UBCHeight -1) 
			&& (iter->second->nHeight < (Params().GetConsensus().UBCHeight + Params().GetConsensus().UBCInitBlockCount -1)))
			return true;	
	}

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);
    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
