# Mini-Ethereum Implementation

A minimal, dual-client (Execution & Consensus) Ethereum blockchain implementation built from scratch in C++17. Designed as a lightweight, clean, and highly traceable simulation framework to study how Ethereum nodes operate under modern Proof-of-Stake (PoS) architecture.

---

## 🏗️ Repository Architecture & Progress

Our implementation strategy enforces a strict dependency flow, starting with foundational shared modules (`common/`) before assembling client engines.

```text
mini-ethereum/
│
├── common/                  ← Core Foundations [IN PROGRESS]
│   ├── rlp/                 • Recursive Length Prefix (Custom impl) [PARTIALLY COMPLETE]
│   ├── ssz/                 • Simple Serialize (Custom impl for Consensus Data)
│   ├── crypto/              • Cryptographic primitives: Custom Keccak256; BLS12-381 & ECDSA via library [KECCAK COMPLETE]
│   ├── types/               • Ethereum Native Types (Address, Hash, uint256, Tx, Block, Receipt) [100% COMPLETE]
│   └── config/              • Network Configurations (genesis.json, chain_id, slot_duration)
│
├── execution_client/        ← Execution Layer (EL)
│   ├── evm/                 • Custom EVM Interpreter (~50 core opcodes, gas metering)
│   ├── state/               • Account state, simplified Merkle Patricia Trie (MPT), LevelDB wrapper
│   ├── txpool/              • Transaction Mempool with EIP-1559 validation
│   └── engine/              • Engine API Handler (CL-EL communication bridge)
│
├── consensus_client/        ← Consensus Layer (CL)
│   ├── beacon/              • Slot/Epoch logic, chain head tracker
│   ├── validator/           • Block production, BLS signing, attestation broadcasting
│   └── fork_choice/         • Naive LMD-GHOST Fork Choice rule
│
└── simulation/              ← Local Cluster Simulation (Deterministic testing framework)
    ├── node.cpp             • Unified Node abstraction (1 EL + 1 CL + 1 Validator)
    ├── fake_network.cpp     • Virtual message bus for deterministic p2p simulation
    └── scenario.cpp         • Multi-node consensus orchestration and tx routing
