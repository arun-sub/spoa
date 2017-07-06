/*!
 * @file spoa_test.cpp
 *
 * @brief Spoa unit test source file
 */

#include "spoa_test_config.h"

#include "chain.hpp"
#include "spoa/spoa.hpp"
#include "gtest/gtest.h"

class SpoaAlignmentTest: public ::testing::Test {
public:
    void SetUp(const std::string& file_name, spoa::AlignmentType alignment_type,
        int8_t match, int8_t mismatch, int8_t gap_open, int8_t gap_extend) {

        reader = bioparser::createReader<spoa::Chain, bioparser::FastqReader>(file_name);
        alignment_engine = spoa::createAlignmentEngine(alignment_type, match,
            mismatch, gap_open, gap_extend);
        graph = spoa::createGraph();
    }

    void TearDown() {}

    void initialize() {
        reader->read_objects(chains, -1);

        size_t max_sequence_size = 0;
        for (const auto& it: chains) {
            max_sequence_size = std::max(max_sequence_size, it->data().size());
        }
        alignment_engine->prealloc(max_sequence_size, 4);
    }

    void construct_partial_order_graph(bool use_qualities) {
        for (const auto& it: chains) {
            auto alignment = alignment_engine->align_sequence_with_graph(
                it->data(), graph);

            if (use_qualities) {
                graph->add_alignment(alignment, it->data(), it->quality());
            } else {
                graph->add_alignment(alignment, it->data());
            }
        }
    }

    std::vector<std::unique_ptr<spoa::Chain>> chains;
    std::unique_ptr<bioparser::Reader<spoa::Chain>> reader;
    std::unique_ptr<spoa::AlignmentEngine> alignment_engine;
    std::unique_ptr<spoa::Graph> graph;
};

TEST(SpoaTest, AlignmentTypeError) {
    EXPECT_DEATH((spoa::createAlignmentEngine(static_cast<spoa::AlignmentType>(4),
        0, 0, 0, 0)), "spoa::createAlignmentEngine error: invalid alignment type");
}

TEST(SpoaTest, EmptyInputError) {
    auto alignment_engine = spoa::createAlignmentEngine(
        static_cast<spoa::AlignmentType>(0), 0, 0, 0, 0);
    auto graph = spoa::createGraph();
    auto alignment = alignment_engine->align_sequence_with_graph("", graph);

    EXPECT_TRUE(alignment.empty());
}

TEST_F(SpoaAlignmentTest, LocalConsensus) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kSW,
        5, -4, -8, -8);

    initialize();
    construct_partial_order_graph(false);

    auto consensus = graph->generate_consensus();

    std::string valid_result = "AATGATGCGCTTTGTTGGCGCGGTGGCTTGATGCAGGGGCTAATCGA"
        "CCTCTGGCAACCACTTTTCCATGACAGGAGTTGAATATGGCATTCAGTAATCCCTTCGATGATCCGCAGG"
        "GAGCGTTTTACATATTGCGCAATGCGCAGGGGCAATTCAGTCTGTGGCCGCAACAATGCGTCTTACCGGC"
        "AGGCTGGGACATTGTGTGTCAGCCGCAGTCACAGGCGTCCTGCCAGCAGTGGCTGGAAGCCCACTGGCGT"
        "ACTCTGACACCGACGAATTTTACCCAGTTGCAGGAGGCACAATGAGCCAGCATTTACCTTTGGTCGCCGC"
        "ACAGCCCGGCATCTGGATGGCAGAAAAACTGTCAGAATTACCCTCCGCCTGGAGCGTGGCGCATTACGTT"
        "GAGTTAACCGGAGAGGTTGATTCGCCATTACTGGCCCGCGCGGTGGTTGCCGGACTAGCGCAAGCAGATA"
        "CGCTTTACACGCGCAACCAAGGATTTCGG";

    EXPECT_TRUE(consensus.compare(valid_result) == 0);
}

TEST_F(SpoaAlignmentTest, LocalConsensusWithQualities) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kSW,
        5, -4, -8, -8);

    initialize();
    construct_partial_order_graph(true);

    auto consensus = graph->generate_consensus();

    std::string valid_result = "AATGATGCGCTTTGTTGGCGCGGTGGCTTGATGCAGGGGCTAATCGA"
        "CCTCTGGCAACCACTTTTCCATGACAGGAGTTGAATATGGCATTCAGTAATCCCTTCGATGATCCGCAGG"
        "GAGCGTTTTACATATTGCGCAATGCGCAGGGGCAATTCAGTCTGTGGCCGCAACAATGCGTCTTACCGGC"
        "AGGCTGGGACATTGTGTGTCAGCCGCAGTCACAGGCGTCCTGCCAGCAGTGGCTGGAAGCCCACTGGCGT"
        "ACTCTGACACCGACGAATTTTACCCAGTTGCAGGAGGCACAATGAGCCAGCATTTACCTTTGGTCGCCGC"
        "ACAGCCCGGCATCTGGATGGCAGAAAAACTGTCAGAATTACCCTCCGCCTGGAGCGTGGCGCATTACGTT"
        "GAGTTAACCGGAGAGGTTGATTCGCCATTACTGGCCCGCGCGGTGGTTGCCGGACTAGCGCAAGCAGATA"
        "CGCTTTACACGCGCAACCAAGGATTTCGG";

    EXPECT_TRUE(consensus.compare(valid_result) == 0);
}

TEST_F(SpoaAlignmentTest, LocalGotohConsensus) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kSW,
        5, -4, -8, -6);

    initialize();
    construct_partial_order_graph(false);

    auto consensus = graph->generate_consensus();

    std::string valid_result = "AATGATGCGCTTTGTTGGCGCGGTGGCTTGATGCAGGGGCTAATCGA"
        "CCTCTGGCAACCACTTTTCCATGACAGGAGTTGAATATGGCATTCAGTAATCCCTTCGATGATCGCAGCA"
        "GGGAGCGTTTTACATATTGCGCAATGCGCAGGGCAATTCAGTCTGTGGCCGCAACAATGCGTCTTACCGG"
        "CAGGCTGGGACATTGTGTGTCAGCCGCAGTCACAGGCGTCCTGCCAGCAGTGGCTGGAAGCCCACTGGCG"
        "TACTCTGACACCGACGAATTTTACCCAGTTGCAGGAGGCACAATGAGCCAGCATTTACCTTTGGTCGCCG"
        "CACAGCCCGGCATCTGGATGGCAGAAAAACTGTCAGAATTACCCTCCGCCTGGAGCGTGGCGCATTACGT"
        "TGAGTTAACCGGAGAGGTTGATTCGCCATTACTGGCCCGCGCGGTGGTTGCCGGACTAGCGCAAGCAGAT"
        "ACGCGCGCAACCAAGGATTTCGG";

    EXPECT_TRUE(consensus.compare(valid_result) == 0);
}

TEST_F(SpoaAlignmentTest, LocalGotohConsensusWithQualities) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kSW,
        5, -4, -8, -6);

    initialize();
    construct_partial_order_graph(true);

    auto consensus = graph->generate_consensus();

    std::string valid_result = "AATGATGCGCTTTGTTGGCGCGGTGGCTTGATGCAGGGGCTAATCGA"
        "CCTCTGGCAACCACTTTTCCATGACAGGAGTTGAATATGGCATTCAGTAATCCCTTCGATGATCCGCAGG"
        "GAGCGTTTTACATATTGCGCAATGCGCAGGGCAATTCAGTCTGTGGCCGCAACAATGCGTCTTACCGGCA"
        "GGCTGGGACATTGTGTGTCAGCCGCAGTCACAGGCGTCCTGCCAGCAGTGGCTGGAAGCCCACTGGCGTA"
        "CTCTGACACCGACGAATTTTACCCAGTTGCAGGAGGCACAATGAGCCAGCATTTACCTTTGGTCGCCGCA"
        "CAGCCCGGCATCTGGATGGCAGAAAAACTGTCAGAATTACCCTCCGCCTGGAGCGTGGCGCATTACGTTG"
        "AGTTAACCGGAGAGGTTGATTCGCCATTACTGGCCCGCGCGGTGGTTGCCGGACTAGCGCAAGCAGATAC"
        "GCGCGCAACCAAGGATTTCGG";

    EXPECT_TRUE(consensus.compare(valid_result) == 0);
}

TEST_F(SpoaAlignmentTest, GlobalConsensus) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kNW,
        5, -4, -8, -8);

    initialize();
    construct_partial_order_graph(false);

    auto consensus = graph->generate_consensus();

    std::string valid_result = "ATGATGCGCTTTGTTGGCGCGGTGGCTTGATGCAGGGGCTAATCGAC"
        "CTCTGGCAACCACTTTTCCATGACAGGAGTTGAATATGGCATTCAGTAATCCCTTCGATGATCCGCAGGG"
        "AGCGTTTTACATATTGCGCAATGCGCAGGGGCAATTCAGTCTGTGGCCGCAACAATGCGTCTTACCGGCA"
        "GGCTGGGACATTGTGTGTCAGCCGCAGTCACAGGCGTCCTGCCAGCAGTGGCTGGAAGCCCACTGGCGTA"
        "CTCTGACACCGACGAATTTTACCCAGTTGCAGGAGGCACAATGAGCCAGCATTTACCTTTGGTCGCCGCA"
        "CAGCCCGGCATCTGGATGGCAGAAAAACTGTCAGAATTACCCTCCGCCTGGAGCGTGGCGCATTACGTTG"
        "AGTTAACCGGAGAGGTTGATTCGCCATTACTGGCCCGCGCGGTGGTTGCCGGACTAGCGCAAGCAGATAC"
        "GC";

    EXPECT_TRUE(consensus.compare(valid_result) == 0);
}

TEST_F(SpoaAlignmentTest, GlobalConsensusWithQualities) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kNW,
        5, -4, -8, -8);

    initialize();
    construct_partial_order_graph(true);

    auto consensus = graph->generate_consensus();

    std::string valid_result = "ATGATGCGCTTTGTTGGCGCGGTGGCTTGATGCAGGGGCTAATCGAC"
        "CTCTGGCAACCACTTTTCCATGACAGGAGTTGAATATGGCATTCAGTAATCCCTTCGATGATCCGCAGGG"
        "AGCGTTTTACATATTGCGCAATGCGCAGGGGCAATTCAGTCTGTGGCCGCAACAATGCGTCTTACCGGCA"
        "GGCTGGGACATTGTGTGTCAGCCGCAGTCACAGGCGTCCTGCCAGCAGTGGCTGGAAGCCCACTGGCGTA"
        "CTCTGACACCGACGAATTTTACCCAGTTGCAGGAGGCACAATGAGCCAGCATTTACCTTTGGTCGCCGCA"
        "CAGCCCGGCATCTGGATGGCAGAAAAACTGTCAGAATTACCCTCCGCCTGGAGCGTGGCGCATTACGTTG"
        "AGTTAACCGGAGAGGTTGATTCGCCATTACTGGCCCGCGCGGTGGTTGCCGGACTAGCGCAAGCAGATAC"
        "GC";

    EXPECT_TRUE(consensus.compare(valid_result) == 0);
}

TEST_F(SpoaAlignmentTest, GlobalGotohConsensus) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kNW,
        5, -4, -8, -6);

    initialize();
    construct_partial_order_graph(false);

    auto consensus = graph->generate_consensus();

    std::string valid_result = "ATGATGCGCTTTGTTGGCGCGGTGGCTTGATGCAGGGGCTAATCGAC"
        "CTCTGGCAACCACTTTTCCATGACAGGAGTTGAATATGGCATTCAGTAATCCCTTCGATGATCGCAGCAG"
        "GGAGCGTTTTACATATTGCGCAATGCGCAGGGCAATTCAGTCTGTGGCCGCAACAATGCGTCTTACCGGC"
        "AGGCTGGGACATTGTGTGTCAGCCGCAGTCACAGGCGTCCTGCCAGCAGTGGCTGGAAGCCCACTGGCGT"
        "ACTCTGACACCGACGAATTTTACCCAGTTGCAGGAGGCACAATGAGCCAGCATTTACCTTTGGTCGCCGC"
        "ACAGCCCGGCATCTGGATGGCAGAAAAACTGTCAGAATTACCCTCCGCCTGGAGCGTGGCGCATTACGTT"
        "GAGTTAACCGGAGAGGTTGATTCGCCATTACTGGCCCGCGCGGTGGTTGCCGGACTAGCGCAAGCAGATA"
        "CGC";

    EXPECT_TRUE(consensus.compare(valid_result) == 0);
}

TEST_F(SpoaAlignmentTest, GlobalGotohConsensusWithQualities) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kNW,
        5, -4, -8, -6);

    initialize();
    construct_partial_order_graph(true);

    auto consensus = graph->generate_consensus();

    std::string valid_result = "ATGATGCGCTTTGTTGGCGCGGTGGCTTGATGCAGGGGCTAATCGAC"
        "CTCTGGCAACCACTTTTCCATGACAGGAGTTGAATATGGCATTCAGTAATCCCTTCGATGATCCGCAGGG"
        "AGCGTTTTACATATTGCGCAATGCGCAGGGCAATTCAGTCTGTGGCCGCAACAATGCGTCTTACCGGCAG"
        "GCTGGGACATTGTGTGTCAGCCGCAGTCACAGGCGTCCTGCCAGCAGTGGCTGGAAGCCCACTGGCGTAC"
        "TCTGACACCGACGAATTTTACCCAGTTGCAGGAGGCACAATGAGCCAGCATTTACCTTTGGTCGCCGCAC"
        "AGCCCGGCATCTGGATGGCAGAAAAACTGTCAGAATTACCCTCCGCCTGGAGCGTGGCGCATTACGTTGA"
        "GTTAACCGGAGAGGTTGATTCGCCATTACTGGCCCGCGCGGTGGTTGCCGGACTAGCGCAAGCAGATACG"
        "C";

    EXPECT_TRUE(consensus.compare(valid_result) == 0);
}

TEST_F(SpoaAlignmentTest, SemiGlobalConsensus) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kOV,
        5, -4, -8, -8);

    initialize();
    construct_partial_order_graph(false);

    auto consensus = graph->generate_consensus();

    std::string valid_result = "ACATGATGCGCTTTGTTGGCGCGGTGGCTTGATGCAGGGGCTAATCG"
        "ACCTCTGGCAACCACTTTTCCATGACAGGAGTTGAATATGGCATTCAGTAATCCCTTCGATGATCCGCAG"
        "GGAGCGTTTTACATATTGCGCAATGCGCAGGGGCAATTCAGTCTGTGGCCGCAACAATGCGTCTTACCGG"
        "CAGGCTGGGACATTGTGTGTCAGCCGCAGTCACAGGCGTCCTGCCAGCAGTGGCTGGAAGCCCACTGGCG"
        "TACTCTGACACCGACGAATTTTACCCAGTTGCAGGAGGCACAATGAGCCAGCATTTACCTTTGGTCGCCG"
        "CACAGCCCGGCATCTGGATGGCAGAAAAACTGTCAGAATTACCCTCCGCCTGGAGCGTGGCGCATTACGT"
        "TGAGTTAACCGGAGAGGTTGATTCGCCATTACTGGCCCGCGCGGTGGTTGCCGGACTAGCGCAAGCAGAT"
        "ACGCGTTTTACACGCGCAACCAAGGATTTCGG";

    EXPECT_TRUE(consensus.compare(valid_result) == 0);
}

TEST_F(SpoaAlignmentTest, SemiGlobalConsensusWithQualities) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kOV,
        5, -4, -8, -8);

    initialize();
    construct_partial_order_graph(true);

    auto consensus = graph->generate_consensus();

    std::string valid_result = "ACATGATGCGCTTTGTTGGCGCGGTGGCTTGATGCAGGGGCTAATCG"
        "ACCTCTGGCAACCACTTTTCCATGACAGGAGTTGAATATGGCATTCAGTAATCCCTTCGATGATCCGCAG"
        "GGAGCGTTTTACATATTGCGCAATGCGCAGGGGCAATTCAGTCTGTGGCCGCAACAATGCGTCTTACCGG"
        "CAGGCTGGGACATTGTGTGTCAGCCGCAGTCACAGGCGTCCTGCCAGCAGTGGCTGGAAGCCCACTGGCG"
        "TACTCTGACACCGACGAATTTTACCCAGTTGCAGGAGGCACAATGAGCCAGCATTTACCTTTGGTCGCCG"
        "CACAGCCCGGCATCTGGATGGCAGAAAAACTGTCAGAATTACCCTCCGCCTGGAGCGTGGCGCATTACGT"
        "TGAGTTAACCGGAGAGGTTGATTCGCCATTACTGGCCCGCGCGGTGGTTGCCGGACTAGCGCAAGCAGAT"
        "ACGCGTTTTACACGCGCAACCAAGGATTTCGG";

    EXPECT_TRUE(consensus.compare(valid_result) == 0);
}

TEST_F(SpoaAlignmentTest, SemiGlobalGotohConsensus) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kOV,
        5, -4, -8, -6);

    initialize();
    construct_partial_order_graph(false);

    auto consensus = graph->generate_consensus();

    std::string valid_result = "ACATGATGCGCTTTGTTGGCGCGGTGGCTTGATGCAGGGGCTAATCG"
        "ACCTCTGGCAACCACTTTTCCATGACAGGAGTTGAATATGGCATTCAGTAATCCCTTCGATGATCGCAGC"
        "AGGGAGCGTTTTACATATTGCGCAATGCGCAGGGCAATTCAGTCTGTGGCCGCAACAATGCGTCTTACCG"
        "GCAGGCTGGGACATTGTGTGTCAGCCGCAGTCACAGGCGTCCTGCCAGCAGTGGCTGGAAGCCCACTGGC"
        "GTACTCTGACACCGACGAATTTTACCCAGTTGCAGGAGGCACAATGAGCCAGCATTTACCTTTGGTCGCC"
        "GCACAGCCCGGCATCTGGATGGCAGAAAAACTGTCAGAATTACCCTCCGCCTGGAGCGTGGCGCATTACG"
        "TTGAGTTAACCGGAGAGGTTGATTCGCCATTACTGGCCCGCGCGGTGGTTGCCGGACTAGCGCAAGCAGA"
        "TACGCCGCGCAACCAAGGATTTCGG";

    EXPECT_TRUE(consensus.compare(valid_result) == 0);
}

TEST_F(SpoaAlignmentTest, SemiGlobalGotohConsensusWithQualities) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kOV,
        5, -4, -8, -6);

    initialize();
    construct_partial_order_graph(true);

    auto consensus = graph->generate_consensus();

    std::string valid_result = "ACATGATGCGCTTTGTTGGCGCGGTGGCTTGATGCAGGGGCTAATCG"
        "ACCTCTGGCAACCACTTTTCCATGACAGGAGTTGAATATGGCATTCAGTAATCCCTTCGATGATCCGCAG"
        "GGAGCGTTTTACATATTGCGCAATGCGCAGGGCAATTCAGTCTGTGGCCGCAACAATGCGTCTTACCGGC"
        "AGGCTGGGACATTGTGTGTCAGCCGCAGTCACAGGCGTCCTGCCAGCAGTGGCTGGAAGCCCACTGGCGT"
        "ACTCTGACACCGACGAATTTTACCCAGTTGCAGGAGGCACAATGAGCCAGCATTTACCTTTGGTCGCCGC"
        "ACAGCCCGGCATCTGGATGGCAGAAAAACTGTCAGAATTACCCTCCGCCTGGAGCGTGGCGCATTACGTT"
        "GAGTTAACCGGAGAGGTTGATTCGCCATTACTGGCCCGCGCGGTGGTTGCCGGACTAGCGCAAGCAGATA"
        "CGCCGCGCAACCAAGGATTTCGG";

    EXPECT_TRUE(consensus.compare(valid_result) == 0);
}

TEST_F(SpoaAlignmentTest, LocalMSA) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kSW,
        5, -4, -8, -8);

    initialize();
    construct_partial_order_graph(false);

    std::vector<std::string> msa;
    graph->generate_multiple_sequence_alignment(msa);

    EXPECT_TRUE(msa.size() == chains.size());

    for (uint32_t i = 0; i < msa.size(); ++i) {
        std::string tmp = "";
        for (const auto& c: msa[i]) {
            if (c != '-') tmp += c;
        }

        EXPECT_TRUE(tmp.size() == chains[i]->data().size());
        EXPECT_TRUE(tmp.compare(chains[i]->data()) == 0);
    }
}

TEST_F(SpoaAlignmentTest, LocalMSAWithQualities) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kSW,
        5, -4, -8, -8);

    initialize();
    construct_partial_order_graph(true);

    std::vector<std::string> msa;
    graph->generate_multiple_sequence_alignment(msa);

    EXPECT_TRUE(msa.size() == chains.size());

    for (uint32_t i = 0; i < msa.size(); ++i) {
        std::string tmp = "";
        for (const auto& c: msa[i]) {
            if (c != '-') tmp += c;
        }

        EXPECT_TRUE(tmp.size() == chains[i]->data().size());
        EXPECT_TRUE(tmp.compare(chains[i]->data()) == 0);
    }
}

TEST_F(SpoaAlignmentTest, LocalGotohMSA) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kSW,
        5, -4, -8, -6);

    initialize();
    construct_partial_order_graph(false);

    std::vector<std::string> msa;
    graph->generate_multiple_sequence_alignment(msa);

    EXPECT_TRUE(msa.size() == chains.size());

    for (uint32_t i = 0; i < msa.size(); ++i) {
        std::string tmp = "";
        for (const auto& c: msa[i]) {
            if (c != '-') tmp += c;
        }

        EXPECT_TRUE(tmp.size() == chains[i]->data().size());
        EXPECT_TRUE(tmp.compare(chains[i]->data()) == 0);
    }
}

TEST_F(SpoaAlignmentTest, LocalGotohMSAWithQualities) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kSW,
        5, -4, -8, -6);

    initialize();
    construct_partial_order_graph(true);

    std::vector<std::string> msa;
    graph->generate_multiple_sequence_alignment(msa);

    EXPECT_TRUE(msa.size() == chains.size());

    for (uint32_t i = 0; i < msa.size(); ++i) {
        std::string tmp = "";
        for (const auto& c: msa[i]) {
            if (c != '-') tmp += c;
        }

        EXPECT_TRUE(tmp.size() == chains[i]->data().size());
        EXPECT_TRUE(tmp.compare(chains[i]->data()) == 0);
    }
}

TEST_F(SpoaAlignmentTest, GlobalMSA) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kNW,
        5, -4, -8, -8);

    initialize();
    construct_partial_order_graph(false);

    std::vector<std::string> msa;
    graph->generate_multiple_sequence_alignment(msa);

    EXPECT_TRUE(msa.size() == chains.size());

    for (uint32_t i = 0; i < msa.size(); ++i) {
        std::string tmp = "";
        for (const auto& c: msa[i]) {
            if (c != '-') tmp += c;
        }

        EXPECT_TRUE(tmp.size() == chains[i]->data().size());
        EXPECT_TRUE(tmp.compare(chains[i]->data()) == 0);
    }
}

TEST_F(SpoaAlignmentTest, GlobalMSAWithQualities) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kNW,
        5, -4, -8, -8);

    initialize();
    construct_partial_order_graph(true);

    std::vector<std::string> msa;
    graph->generate_multiple_sequence_alignment(msa);

    EXPECT_TRUE(msa.size() == chains.size());

    for (uint32_t i = 0; i < msa.size(); ++i) {
        std::string tmp = "";
        for (const auto& c: msa[i]) {
            if (c != '-') tmp += c;
        }

        EXPECT_TRUE(tmp.size() == chains[i]->data().size());
        EXPECT_TRUE(tmp.compare(chains[i]->data()) == 0);
    }
}

TEST_F(SpoaAlignmentTest, GlobalGotohMSA) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kNW,
        5, -4, -8, -6);

    initialize();
    construct_partial_order_graph(false);

    std::vector<std::string> msa;
    graph->generate_multiple_sequence_alignment(msa);

    EXPECT_TRUE(msa.size() == chains.size());

    for (uint32_t i = 0; i < msa.size(); ++i) {
        std::string tmp = "";
        for (const auto& c: msa[i]) {
            if (c != '-') tmp += c;
        }

        EXPECT_TRUE(tmp.size() == chains[i]->data().size());
        EXPECT_TRUE(tmp.compare(chains[i]->data()) == 0);
    }
}

TEST_F(SpoaAlignmentTest, GlobalGotohMSAWithQualities) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kNW,
        5, -4, -8, -6);

    initialize();
    construct_partial_order_graph(true);

    std::vector<std::string> msa;
    graph->generate_multiple_sequence_alignment(msa);

    EXPECT_TRUE(msa.size() == chains.size());

    for (uint32_t i = 0; i < msa.size(); ++i) {
        std::string tmp = "";
        for (const auto& c: msa[i]) {
            if (c != '-') tmp += c;
        }

        EXPECT_TRUE(tmp.size() == chains[i]->data().size());
        EXPECT_TRUE(tmp.compare(chains[i]->data()) == 0);
    }
}

TEST_F(SpoaAlignmentTest, SemiGlobalMSA) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kOV,
        5, -4, -8, -8);

    initialize();
    construct_partial_order_graph(false);

    std::vector<std::string> msa;
    graph->generate_multiple_sequence_alignment(msa);

    EXPECT_TRUE(msa.size() == chains.size());

    for (uint32_t i = 0; i < msa.size(); ++i) {
        std::string tmp = "";
        for (const auto& c: msa[i]) {
            if (c != '-') tmp += c;
        }

        EXPECT_TRUE(tmp.size() == chains[i]->data().size());
        EXPECT_TRUE(tmp.compare(chains[i]->data()) == 0);
    }
}

TEST_F(SpoaAlignmentTest, SemiGlobalMSAWithQualities) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kOV,
        5, -4, -8, -8);

    initialize();
    construct_partial_order_graph(true);

    std::vector<std::string> msa;
    graph->generate_multiple_sequence_alignment(msa);

    EXPECT_TRUE(msa.size() == chains.size());

    for (uint32_t i = 0; i < msa.size(); ++i) {
        std::string tmp = "";
        for (const auto& c: msa[i]) {
            if (c != '-') tmp += c;
        }

        EXPECT_TRUE(tmp.size() == chains[i]->data().size());
        EXPECT_TRUE(tmp.compare(chains[i]->data()) == 0);
    }
}

TEST_F(SpoaAlignmentTest, SemiGlobalGotohMSA) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kOV,
        5, -4, -8, -6);

    initialize();
    construct_partial_order_graph(false);

    std::vector<std::string> msa;
    graph->generate_multiple_sequence_alignment(msa);

    EXPECT_TRUE(msa.size() == chains.size());

    for (uint32_t i = 0; i < msa.size(); ++i) {
        std::string tmp = "";
        for (const auto& c: msa[i]) {
            if (c != '-') tmp += c;
        }

        EXPECT_TRUE(tmp.size() == chains[i]->data().size());
        EXPECT_TRUE(tmp.compare(chains[i]->data()) == 0);
    }
}

TEST_F(SpoaAlignmentTest, SemiGlobalGotohMSAWithQualities) {
    SetUp(spoa_test_data_path + "sample.fastq", spoa::AlignmentType::kOV,
        5, -4, -8, -6);

    initialize();
    construct_partial_order_graph(true);

    std::vector<std::string> msa;
    graph->generate_multiple_sequence_alignment(msa);

    EXPECT_TRUE(msa.size() == chains.size());

    for (uint32_t i = 0; i < msa.size(); ++i) {
        std::string tmp = "";
        for (const auto& c: msa[i]) {
            if (c != '-') tmp += c;
        }

        EXPECT_TRUE(tmp.size() == chains[i]->data().size());
        EXPECT_TRUE(tmp.compare(chains[i]->data()) == 0);
    }
}
