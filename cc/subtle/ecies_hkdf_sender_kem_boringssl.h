// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TINK_SUBTLE_ECIES_HKDF_SENDER_KEM_BORINGSSL_H_
#define TINK_SUBTLE_ECIES_HKDF_SENDER_KEM_BORINGSSL_H_

#include "absl/strings/string_view.h"
#include "openssl/curve25519.h"
#include "openssl/ec.h"
#include "tink/internal/fips_utils.h"
#include "tink/internal/ssl_unique_ptr.h"
#include "tink/subtle/common_enums.h"
#include "tink/util/secret_data.h"
#include "tink/util/statusor.h"

namespace crypto {
namespace tink {
namespace subtle {

// HKDF-based KEM (key encapsulation mechanism) for ECIES sender,
// using Boring SSL for the underlying cryptographic operations.
class EciesHkdfSenderKemBoringSsl {
 public:
  // Container for data of keys generated by the KEM.
  class KemKey {
   public:
    KemKey() = default;
    explicit KemKey(std::string kem_bytes, util::SecretData symmetric_key)
        : kem_bytes_(std::move(kem_bytes)),
          symmetric_key_(std::move(symmetric_key)) {}

    const std::string& get_kem_bytes() const { return kem_bytes_; }

    const util::SecretData& get_symmetric_key() const { return symmetric_key_; }

   private:
    std::string kem_bytes_;
    util::SecretData symmetric_key_;
  };

  // Constructs a sender KEM for the specified curve and recipient's
  // public key point.  The public key's coordinates are big-endian byte array.
  static crypto::tink::util::StatusOr<
      std::unique_ptr<const EciesHkdfSenderKemBoringSsl>>
  New(EllipticCurveType curve, const std::string& pubx,
      const std::string& puby);

  // Generates ephemeral key pairs, computes ECDH's shared secret based on
  // generated ephemeral key and recipient's public key, then uses HKDF
  // to derive the symmetric key from the shared secret, 'hkdf_info' and
  // hkdf_salt.
  virtual crypto::tink::util::StatusOr<std::unique_ptr<const KemKey>>
  GenerateKey(HashType hash, absl::string_view hkdf_salt,
              absl::string_view hkdf_info, uint32_t key_size_in_bytes,
              EcPointFormat point_format) const = 0;

  virtual ~EciesHkdfSenderKemBoringSsl() = default;
};

// Implementation of EciesHkdfSenderKemBoringSsl for the NIST P-curves.
class EciesHkdfNistPCurveSendKemBoringSsl : public EciesHkdfSenderKemBoringSsl {
 public:
  // Constructs a sender KEM for the specified curve and recipient's
  // public key point.  The public key's coordinates are big-endian byte array.
  static crypto::tink::util::StatusOr<
      std::unique_ptr<const EciesHkdfSenderKemBoringSsl>>
  New(EllipticCurveType curve, const std::string& pubx,
      const std::string& puby);

  // Generates ephemeral key pairs, computes ECDH's shared secret based on
  // generated ephemeral key and recipient's public key, then uses HKDF
  // to derive the symmetric key from the shared secret, 'hkdf_info' and
  // hkdf_salt.
  crypto::tink::util::StatusOr<std::unique_ptr<const KemKey>> GenerateKey(
      HashType hash, absl::string_view hkdf_salt, absl::string_view hkdf_info,
      uint32_t key_size_in_bytes, EcPointFormat point_format) const override;

  static constexpr crypto::tink::internal::FipsCompatibility kFipsStatus =
      crypto::tink::internal::FipsCompatibility::kNotFips;

 private:
  EciesHkdfNistPCurveSendKemBoringSsl(EllipticCurveType curve,
                                      const std::string& pubx,
                                      const std::string& puby,
                                      EC_POINT* peer_pub_key);

  EllipticCurveType curve_;
  std::string pubx_;
  std::string puby_;
  internal::SslUniquePtr<EC_POINT> peer_pub_key_;
};

// Implementation of EciesHkdfSenderKemBoringSsl for curve25519.
class EciesHkdfX25519SendKemBoringSsl : public EciesHkdfSenderKemBoringSsl {
 public:
  // Constructs a sender KEM for the specified curve and recipient's
  // public key point.  The public key's coordinates are big-endian byte array.
  static crypto::tink::util::StatusOr<
      std::unique_ptr<const EciesHkdfSenderKemBoringSsl>>
  New(EllipticCurveType curve, const std::string& pubx,
      const std::string& puby);

  // Generates ephemeral key pairs, computes ECDH's shared secret based on
  // generated ephemeral key and recipient's public key, then uses HKDF
  // to derive the symmetric key from the shared secret, 'hkdf_info' and
  // hkdf_salt.
  crypto::tink::util::StatusOr<std::unique_ptr<const KemKey>> GenerateKey(
      HashType hash, absl::string_view hkdf_salt, absl::string_view hkdf_info,
      uint32_t key_size_in_bytes, EcPointFormat point_format) const override;

  static constexpr crypto::tink::internal::FipsCompatibility kFipsStatus =
      crypto::tink::internal::FipsCompatibility::kNotFips;

 private:
  explicit EciesHkdfX25519SendKemBoringSsl(
      const std::string& peer_public_value);

  uint8_t peer_public_value_[X25519_PUBLIC_VALUE_LEN];
};

}  // namespace subtle
}  // namespace tink
}  // namespace crypto

#endif  // TINK_SUBTLE_ECIES_HKDF_SENDER_KEM_BORINGSSL_H_
