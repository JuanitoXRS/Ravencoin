// Copyright (c) 2017 The Raven Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef RAVENCOIN_ASSET_PROTOCOL_H
#define RAVENCOIN_ASSET_PROTOCOL_H

#include <amount.h>

#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include <list>
#include <tinyformat.h>
#include "serialize.h"
#include "primitives/transaction.h"
#include "assettypes.h"

#define RVN_R 114
#define RVN_V 118
#define RVN_N 110
#define RVN_Q 113
#define RVN_T 116
#define RVN_O 111

#define OWNER "!"
#define OWNER_LENGTH 1
#define MIN_ASSET_LENGTH 3
#define MAX_ASSET_LENGTH 31
#define OWNER_ASSET_AMOUNT 1 * COIN

class CScript;
class CDataStream;
class CTransaction;
class CTxOut;
class Coin;

// 50000 * 82 Bytes == 4.1 Mb
#define MAX_CACHE_ASSETS_SIZE 50000

class CAssets {
public:
    std::map<std::string, std::set<COutPoint> > mapMyUnspentAssets; // Asset Name -> COutPoint
    std::map<std::string, std::set<std::string> > mapAssetsAddresses; // Asset Name -> set <Addresses>
    std::map<std::pair<std::string, std::string>, CAmount> mapAssetsAddressAmount; // pair < Asset Name , Address > -> Quantity of tokens in the address

    std::map<std::string, CNewAsset> mapReissuedAssetData; // Asset Name -> New Asset Data

    CAssets(const CAssets& assets) {
        this->mapMyUnspentAssets = assets.mapMyUnspentAssets;
        this->mapAssetsAddressAmount = assets.mapAssetsAddressAmount;
        this->mapAssetsAddresses = assets.mapAssetsAddresses;
        this->mapReissuedAssetData = assets.mapReissuedAssetData;
    }

    CAssets& operator=(const CAssets& other) {
        mapMyUnspentAssets = other.mapMyUnspentAssets;
        mapAssetsAddressAmount = other.mapAssetsAddressAmount;
        mapAssetsAddresses = other.mapAssetsAddresses;
        mapReissuedAssetData = other.mapReissuedAssetData;
        return *this;
    }

    CAssets() {
        SetNull();
    }

    void SetNull() {
        mapMyUnspentAssets.clear();
        mapAssetsAddresses.clear();
        mapAssetsAddressAmount.clear();
        mapReissuedAssetData.clear();
    }
};

class CAssetsCache : public CAssets
{
private:
    bool AddBackSpentAsset(const Coin& coin, const std::string& assetName, const std::string& address, const CAmount& nAmount, const COutPoint& out);
    void AddToAssetBalance(const std::string& strName, const std::string& address, const CAmount& nAmount);
    bool UndoTransfer(const CAssetTransfer& transfer, const std::string& address, const COutPoint& outToRemove);
public :
    //! These are memory only containers that show dirty entries that will be databased when flushed
    std::vector<CAssetCacheUndoAssetAmount> vUndoAssetAmount;
    std::vector<CAssetCacheSpendAsset> vSpentAssets;
    std::set<std::string> setChangeOwnedOutPoints;

    // New Assets Caches
    std::set<CAssetCacheNewAsset> setNewAssetsToRemove;
    std::set<CAssetCacheNewAsset> setNewAssetsToAdd;

    // New Reissue Caches
    std::set<CAssetCacheReissueAsset> setNewReissueToRemove;
    std::set<CAssetCacheReissueAsset> setNewReissueToAdd;

    // Ownership Assets Caches
    std::set<CAssetCacheNewOwner> setNewOwnerAssetsToAdd;
    std::set<CAssetCacheNewOwner> setNewOwnerAssetsToRemove;

    // Transfer Assets Caches
    std::set<CAssetCacheNewTransfer> setNewTransferAssetsToAdd;
    std::set<CAssetCacheNewTransfer> setNewTransferAssetsToRemove;

    //! During init, the wallet isn't enabled, there for if we disconnect any blocks
    //! We need to store possible unspends of assets that could be ours and check back when the wallet is enabled.
    std::set<CAssetCachePossibleMine> setPossiblyMineAdd;
    std::set<CAssetCachePossibleMine> setPossiblyMineRemove;

    CAssetsCache()
    {
        SetNull();
    }

    CAssetsCache(const CAssetsCache& cache)
    {
        this->mapMyUnspentAssets = cache.mapMyUnspentAssets;
        this->mapAssetsAddressAmount = cache.mapAssetsAddressAmount;
        this->mapAssetsAddresses = cache.mapAssetsAddresses;
        this->mapReissuedAssetData = cache.mapReissuedAssetData;

        // Copy dirty cache also
        this->vSpentAssets = cache.vSpentAssets;
        this->vUndoAssetAmount = cache.vUndoAssetAmount;

        // Transfer Caches
        this->setNewTransferAssetsToAdd = cache.setNewTransferAssetsToAdd;
        this->setNewTransferAssetsToRemove = cache.setNewTransferAssetsToRemove;

        // Issue Caches
        this->setNewAssetsToRemove = cache.setNewAssetsToRemove;
        this->setNewAssetsToAdd = cache.setNewAssetsToAdd;

        // Reissue Caches
        this->setNewReissueToRemove = cache.setNewReissueToRemove;
        this->setNewReissueToAdd = cache.setNewReissueToAdd;

        // Owner Caches
        this->setNewOwnerAssetsToAdd = cache.setNewOwnerAssetsToAdd;
        this->setNewOwnerAssetsToRemove = cache.setNewOwnerAssetsToRemove;

        // Changed Outpoints Caches
        this->setChangeOwnedOutPoints = cache.setChangeOwnedOutPoints;
    }

    CAssetsCache& operator=(const CAssetsCache& cache)
    {
        this->mapMyUnspentAssets = cache.mapMyUnspentAssets;
        this->mapAssetsAddressAmount = cache.mapAssetsAddressAmount;
        this->mapAssetsAddresses = cache.mapAssetsAddresses;
        this->mapReissuedAssetData = cache.mapReissuedAssetData;

        // Copy dirty cache also
        this->vSpentAssets = cache.vSpentAssets;
        this->vUndoAssetAmount = cache.vUndoAssetAmount;

        // Transfer Caches
        this->setNewTransferAssetsToAdd = cache.setNewTransferAssetsToAdd;
        this->setNewTransferAssetsToRemove = cache.setNewTransferAssetsToRemove;

        // Issue Caches
        this->setNewAssetsToRemove = cache.setNewAssetsToRemove;
        this->setNewAssetsToAdd = cache.setNewAssetsToAdd;

        // Reissue Caches
        this->setNewReissueToRemove = cache.setNewReissueToRemove;
        this->setNewReissueToAdd = cache.setNewReissueToAdd;

        // Owner Caches
        this->setNewOwnerAssetsToAdd = cache.setNewOwnerAssetsToAdd;
        this->setNewOwnerAssetsToRemove = cache.setNewOwnerAssetsToRemove;

        // Changed Outpoints Caches
        this->setChangeOwnedOutPoints = cache.setChangeOwnedOutPoints;

        return *this;
    }

    void Copy(const CAssetsCache& cache)
    {
        this->mapMyUnspentAssets = cache.mapMyUnspentAssets;
        this->mapAssetsAddressAmount = cache.mapAssetsAddressAmount;
        this->mapAssetsAddresses = cache.mapAssetsAddresses;
        this->mapReissuedAssetData = cache.mapReissuedAssetData;

        // Copy dirty cache also
        this->vSpentAssets = cache.vSpentAssets;
        this->vUndoAssetAmount = cache.vUndoAssetAmount;

        // Transfer Caches
        this->setNewTransferAssetsToAdd = cache.setNewTransferAssetsToAdd;
        this->setNewTransferAssetsToRemove = cache.setNewTransferAssetsToRemove;

        // Issue Caches
        this->setNewAssetsToRemove = cache.setNewAssetsToRemove;
        this->setNewAssetsToAdd = cache.setNewAssetsToAdd;

        // Reissue Caches
        this->setNewReissueToRemove = cache.setNewReissueToRemove;
        this->setNewReissueToAdd = cache.setNewReissueToAdd;

        // Owner Caches
        this->setNewOwnerAssetsToAdd = cache.setNewOwnerAssetsToAdd;
        this->setNewOwnerAssetsToRemove = cache.setNewOwnerAssetsToRemove;

        // Changed Outpoints Caches
        this->setChangeOwnedOutPoints = cache.setChangeOwnedOutPoints;

        // Copy sets of possibilymine
        this->setPossiblyMineAdd = cache.setPossiblyMineAdd;
        this->setPossiblyMineRemove = cache.setPossiblyMineRemove;
    }

    // Cache only undo functions
    bool RemoveNewAsset(const CNewAsset& asset, const std::string address);
    bool RemoveTransfer(const CAssetTransfer& transfer, const std::string& address, const COutPoint& out);
    bool RemoveOwnerAsset(const std::string& assetsName, const std::string address);
    bool RemoveReissueAsset(const CReissueAsset& reissue, const std::string address, const COutPoint& out, const std::vector<std::pair<std::string, std::string> >& vUndoIPFS);
    bool UndoAssetCoin(const Coin& coin, const COutPoint& out);

    // Cache only add asset functions
    bool AddNewAsset(const CNewAsset& asset, const std::string address);
    bool AddTransferAsset(const CAssetTransfer& transferAsset, const std::string& address, const COutPoint& out, const CTxOut& txOut);
    bool AddOwnerAsset(const std::string& assetsName, const std::string address);
    bool AddToMyUpspentOutPoints(const std::string& strName, const COutPoint& out);
    bool AddReissueAsset(const CReissueAsset& reissue, const std::string address, const COutPoint& out);

    // Cache only validation functions
    bool TrySpendCoin(const COutPoint& out, const CTxOut& coin);

    // Help functions
    bool GetAssetsOutPoints(const std::string& strName, std::set<COutPoint>& outpoints);
    bool ContainsAsset(const CNewAsset& asset);
    bool ContainsAsset(const std::string& assetName);
    bool AddPossibleOutPoint(const CAssetCachePossibleMine& possibleMine);

    bool CheckIfAssetExists(const std::string& name);
    bool GetAssetIfExists(const std::string& name, CNewAsset& asset);

    //! Calculate the size of the CAssets (in bytes)
    size_t DynamicMemoryUsage() const;

    //! Get the size of the none databased cache
    size_t GetCacheSize() const;

    //! Flush a cache to a different cache (usually passets), save to database if fToDataBase is true
    bool Flush(bool fSoftCopy = false, bool fToDataBase = false);

    void ClearDirtyCache() {

        vUndoAssetAmount.clear();
        vSpentAssets.clear();

        setNewAssetsToRemove.clear();
        setNewAssetsToAdd.clear();

        setNewReissueToAdd.clear();
        setNewReissueToRemove.clear();

        setNewTransferAssetsToAdd.clear();
        setNewTransferAssetsToRemove.clear();

        setNewOwnerAssetsToAdd.clear();
        setNewOwnerAssetsToRemove.clear();

        setChangeOwnedOutPoints.clear();

        mapReissuedAssetData.clear();

        // Copy sets of possibilymine
        setPossiblyMineAdd.clear();
        setPossiblyMineRemove.clear();
    }

   std::string CacheToString() const {

      return strprintf("vNewAssetsToRemove size : %d, vNewAssetsToAdd size : %d, vNewTransfer size : %d, vSpentAssets : %d, setChangeOwnedOutPoints size : %d\n",
                       setNewAssetsToRemove.size(), setNewAssetsToAdd.size(), setNewTransferAssetsToAdd.size(), vSpentAssets.size(), setChangeOwnedOutPoints.size());
    }
};

// Functions to be used to get access to the current burn amount required for specific asset issuance transactions
CAmount GetIssueAssetBurnAmount();
CAmount GetReissueAssetBurnAmount();
CAmount GetIssueSubAssetBurnAmount();
CAmount GetIssueUniqueAssetBurnAmount();

bool IsAssetNameValid(const std::string& name);
bool IsAssetNameAnOwner(const std::string& name);

bool IsAssetNameSizeValid(const std::string& name);

bool IsAssetUnitsValid(const CAmount& units);

bool AssetFromTransaction(const CTransaction& tx, CNewAsset& asset, std::string& strAddress);
bool OwnerFromTransaction(const CTransaction& tx, std::string& ownerName, std::string& strAddress);
bool ReissueAssetFromTransaction(const CTransaction& tx, CReissueAsset& reissue, std::string& strAddress);

bool TransferAssetFromScript(const CScript& scriptPubKey, CAssetTransfer& assetTransfer, std::string& strAddress);
bool AssetFromScript(const CScript& scriptPubKey, CNewAsset& assetTransfer, std::string& strAddress);
bool OwnerAssetFromScript(const CScript& scriptPubKey, std::string& assetName, std::string& strAddress);
bool ReissueAssetFromScript(const CScript& scriptPubKey, CReissueAsset& reissue, std::string& strAddress);

bool CheckIssueBurnTx(const CTxOut& txOut);
bool CheckReissueBurnTx(const CTxOut& txOut);

bool CheckIssueDataTx(const CTxOut& txOut);
bool CheckOwnerDataTx(const CTxOut& txOut);
bool CheckReissueDataTx(const CTxOut& txOut);
bool CheckTransferOwnerTx(const CTxOut& txOut);

bool IsScriptNewAsset(const CScript& scriptPubKey);
bool IsScriptOwnerAsset(const CScript& scriptPubKey);
bool IsScriptReissueAsset(const CScript& scriptPubKey);
bool IsScriptTransferAsset(const CScript& scriptPubKey);

bool IsNewOwnerTxValid(const CTransaction& tx, const std::string& assetName, const std::string& address, std::string& errorMsg);

bool CheckAssetOwner(const std::string& assetName);

void UpdatePossibleAssets();

bool GetAssetFromCoin(const Coin& coin, std::string& strName, CAmount& nAmount);
#endif //RAVENCOIN_ASSET_PROTOCOL_H
