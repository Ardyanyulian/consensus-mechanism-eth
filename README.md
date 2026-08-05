<h1 align="center">Consensus Mechanism ETH</h1>

<p align="center">
  <strong>Engine Proof-of-Stake (Gasper) Ethereum berbasis C++17</strong><br>
  Deterministik dan tanpa dependensi eksternal.
</p>

<p align="center">
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-blue.svg" alt="License: MIT" /></a>
  <a href="https://en.cppreference.com/w/cpp/17"><img src="https://img.shields.io/badge/C++-17-blue.svg" alt="C++17" /></a>
</p>

---

## Ringkasan Proyek

**consensus-mechanism-eth** adalah implementasi *bare-metal* dari mekanisme konsensus Ethereum (Gasper). Proyek ini menggabungkan aturan *Fork Choice* **LMD-GHOST** dan *Finality Engine* **Casper FFG** dalam satu biner C++17 yang independen tanpa *framework* atau pustaka kriptografi pihak ketiga.

Proyek ini merujuk langsung pada spesifikasi akademis konsensus Ethereum:
* 📄 **Casper FFG Paper:** [ArXiv:1710.09437](https://arxiv.org/pdf/1710.09437)
* 📄 **Combining Casper & GHOST (Gasper):** [ArXiv:2003.03052](https://arxiv.org/pdf/2003.03052)

---

## Karakteristik Utama

* **Zero External Dependencies:** Implementasi Keccak-256 dan RLP encoding ditulis dari awal (*pure native C++*).
* **Deterministik:** *State machine* murni tanpa *side-effects* non-deterministik, cocok untuk pengujian spesifikasi.
* **Arsitektur Modular:** Pemisahan yang jelas antara kriptografi, RLP serialization, dan logika konsensus.

---

## Struktur Proyek & Arsitektur

```text
consensus-mechanism-eth/
├── common/                  # Primitif & Utilitas
│   ├── crypto/              # Keccak-256 Native Engine
│   ├── rlp/                 # Parser & Encoder RLP
│   └── types/               # Tipe Data Utama (Slot, Epoch, Hash, Validator)
│
├── consensus/               # Engine Proof-of-Stake (Gasper)
│   ├── state.h / .cpp       # BeaconState Machine
│   ├── lmd_ghost.h / .cpp   # Algoritma Fork Choice LMD-GHOST
│   └── casper_ffg.h / .cpp  # Justifikasi & Finalisasi Casper FFG
│
└── test/                    # Suite Pengujian Integrasi
