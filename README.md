Struktur
mini-ethereum/
│
├── common/                  ← Kerjakan INI PERTAMA, 100%
│   ├── rlp/                 (encode/decode — impl sendiri, ~300 baris) (untuk rlp baru sebagian)
│   ├── ssz/                 (encode/decode — impl sendiri, ~200 baris)
│   ├── crypto/              (keccak256 sendiri; BLS12-381 & ECDSA pakai lib) (Untuk keccak256 sudah saya buat manual dan untuk BLS12-381 dan ECDSA itu pakai library saja)
│   ├── types/               (Address, Hash, uint256, Transaction, Block, Receipt) (sudah semua)
│   └── config/              (genesis.json, chain_id, slot_duration)
│
├── execution_client/
│   ├── evm/                 (interpreter + ~50 core opcode dulu, gas meter)
│   ├── state/               (account state, simplified MPT, LevelDB wrapper)
│   ├── txpool/              (mempool, EIP-1559 fee validation)
│   └── engine/              (Engine API handler — terima perintah dari CL)
│
├── consensus_client/
│   ├── beacon/              (slot/epoch logic, chain head tracker)
│   ├── validator/           (propose blok, BLS signing, kirim attestation)
│   └── fork_choice/         (LMD-GHOST — bisa dimulai versi naif dulu)
│
└── simulation/              ← HARUS ADA, ini kunci "low cortisol"
    ├── node.cpp             (satu Node = 1 EL + 1 CL + 1 Validator)
    ├── fake_network.cpp     (message bus: broadcast, unicast antar node)
    └── scenario.cpp         (jalankan 3 node, kirim tx, lihat konsensus)
