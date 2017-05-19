#pragma once

#include "ensdf_records.h"
#include "ensdf_basic_decay.h"

#include "comment_record.h"
#include "history_record.h"

class DaughterParser
{
  struct LevelIndex
  {
    void find(BlockIndices alpos,
              std::string dNucid1,
              std::string dsid,
              const std::vector<std::string>& data);

    void insertAdoptedLevelsBlock(const BlockIndices &newblock,
                                  const std::string& dsid,
                                  const std::vector<std::string>& data);

    std::map<Energy, BlockIndices> adoptblocks;
    // maps DSID to DSSYM (single letter)
    std::map<std::string, std::string> xrefs;

  };

public:
  DaughterParser();
  DaughterParser(uint16_t A, std::string directory);

  std::list<NuclideId> daughters() const;
  std::list<std::string> decays(NuclideId daughter) const;

  DecayScheme get_decay(NuclideId daughter, std::string decay_name) const;

private:
  std::vector<std::string> raw_contents_;
  std::list<HistoryRecord> mass_history_;
  std::list<CommentsRecord> mass_comments_;
  std::map<std::string, std::string> references_;
  std::map<NuclideId, NuclideData> nuclide_data_;

//  std::map<NuclideId, BlockIndices> adopted_levels_; // daughter coordinates
//  std::map<NuclideId, std::map<std::string, DecayData > > decays_; // daughter coordinates: (decay name: basic data)

  static UncertainDouble parseEnsdfMixing(const std::string &s,
                                          const std::string &multipolarity);

  static double norm(std::string rec, double def_value);
  void parseBlocks();
  std::list<BlockIndices> find_blocks() const;

  void parse_comments_block(BlockIndices block_idx);
  void parse_reference_block(BlockIndices block_idx);
};


class ENSDFParser
{
public:
  ENSDFParser();
  ENSDFParser(std::string directory);

  bool good() const;

  std::list<uint16_t> masses() const;
  std::string directory() const;

  DaughterParser get_dp(uint16_t a);

private:
  std::string directory_;
  std::set<uint16_t> masses_;

  std::map<uint16_t, DaughterParser> cache_;
};
