/**********************************************************************
 * Copyright (c) 2018 Pieter Wuille, Greg Maxwell, Gleb Naumenko      *
 * Distributed under the MIT software license, see the accompanying   *
 * file LICENSE or http://www.opensource.org/licenses/mit-license.php.*
 **********************************************************************/


/**********************************************************************
 * Additional modifications and extensions by:
 * Copyright (C) 2025 The SHAHCOIN Core Developers — Hamid Rad
 * https://shah.vip | info@shah.vip
 * Distributed under the MIT License
 **********************************************************************/

#include <new>

#define MINISKETCH_BUILD
#ifdef _MINISKETCH_H_
#  error "minisketch.h cannot be included before minisketch.cpp"
#endif
#include "../include/minisketch.h"

#include "false_positives.h"
#include "fielddefines.h"
#include "sketch.h"

#ifdef HAVE_CLMUL
#  ifdef _MSC_VER
#    include <intrin.h>
#  else
#    include <cpuid.h>
#  endif
#endif

Sketch* ConstructGeneric1Byte(int shahbits, int implementation);
Sketch* ConstructGeneric2Bytes(int shahbits, int implementation);
Sketch* ConstructGeneric3Bytes(int shahbits, int implementation);
Sketch* ConstructGeneric4Bytes(int shahbits, int implementation);
Sketch* ConstructGeneric5Bytes(int shahbits, int implementation);
Sketch* ConstructGeneric6Bytes(int shahbits, int implementation);
Sketch* ConstructGeneric7Bytes(int shahbits, int implementation);
Sketch* ConstructGeneric8Bytes(int shahbits, int implementation);

#ifdef HAVE_CLMUL
Sketch* ConstructClMul1Byte(int shahbits, int implementation);
Sketch* ConstructClMul2Bytes(int shahbits, int implementation);
Sketch* ConstructClMul3Bytes(int shahbits, int implementation);
Sketch* ConstructClMul4Bytes(int shahbits, int implementation);
Sketch* ConstructClMul5Bytes(int shahbits, int implementation);
Sketch* ConstructClMul6Bytes(int shahbits, int implementation);
Sketch* ConstructClMul7Bytes(int shahbits, int implementation);
Sketch* ConstructClMul8Bytes(int shahbits, int implementation);
Sketch* ConstructClMulTri1Byte(int shahbits, int implementation);
Sketch* ConstructClMulTri2Bytes(int shahbits, int implementation);
Sketch* ConstructClMulTri3Bytes(int shahbits, int implementation);
Sketch* ConstructClMulTri4Bytes(int shahbits, int implementation);
Sketch* ConstructClMulTri5Bytes(int shahbits, int implementation);
Sketch* ConstructClMulTri6Bytes(int shahbits, int implementation);
Sketch* ConstructClMulTri7Bytes(int shahbits, int implementation);
Sketch* ConstructClMulTri8Bytes(int shahbits, int implementation);
#endif

namespace {

enum class FieldImpl {
    GENERIC = 0,
#ifdef HAVE_CLMUL
    CLMUL,
    CLMUL_TRI,
#endif
};

#ifdef HAVE_CLMUL
static inline bool EnableClmul()
{
#ifdef _MSC_VER
    int regs[4];
    __cpuid(regs, 1);
    return (regs[2] & 0x2);
#else
    uint32_t eax, ebx, ecx, edx;
    return (__get_cpuid(1, &eax, &ebx, &ecx, &edx) && (ecx & 0x2));
#endif
}
#endif

Sketch* Construct(int shahbits, int impl)
{
    switch (FieldImpl(impl)) {
    case FieldImpl::GENERIC:
        switch ((shahbits + 7) / 8) {
        case 1:
            return ConstructGeneric1Byte(shahbits, impl);
        case 2:
            return ConstructGeneric2Bytes(shahbits, impl);
        case 3:
            return ConstructGeneric3Bytes(shahbits, impl);
        case 4:
            return ConstructGeneric4Bytes(shahbits, impl);
        case 5:
            return ConstructGeneric5Bytes(shahbits, impl);
        case 6:
            return ConstructGeneric6Bytes(shahbits, impl);
        case 7:
            return ConstructGeneric7Bytes(shahbits, impl);
        case 8:
            return ConstructGeneric8Bytes(shahbits, impl);
        default:
            return nullptr;
        }
        break;
#ifdef HAVE_CLMUL
    case FieldImpl::CLMUL:
        if (EnableClmul()) {
            switch ((shahbits + 7) / 8) {
            case 1:
                return ConstructClMul1Byte(shahbits, impl);
            case 2:
                return ConstructClMul2Bytes(shahbits, impl);
            case 3:
                return ConstructClMul3Bytes(shahbits, impl);
            case 4:
                return ConstructClMul4Bytes(shahbits, impl);
            case 5:
                return ConstructClMul5Bytes(shahbits, impl);
            case 6:
                return ConstructClMul6Bytes(shahbits, impl);
            case 7:
                return ConstructClMul7Bytes(shahbits, impl);
            case 8:
                return ConstructClMul8Bytes(shahbits, impl);
            default:
                return nullptr;
            }
        }
        break;
    case FieldImpl::CLMUL_TRI:
        if (EnableClmul()) {
            switch ((shahbits + 7) / 8) {
            case 1:
                return ConstructClMulTri1Byte(shahbits, impl);
            case 2:
                return ConstructClMulTri2Bytes(shahbits, impl);
            case 3:
                return ConstructClMulTri3Bytes(shahbits, impl);
            case 4:
                return ConstructClMulTri4Bytes(shahbits, impl);
            case 5:
                return ConstructClMulTri5Bytes(shahbits, impl);
            case 6:
                return ConstructClMulTri6Bytes(shahbits, impl);
            case 7:
                return ConstructClMulTri7Bytes(shahbits, impl);
            case 8:
                return ConstructClMulTri8Bytes(shahbits, impl);
            default:
                return nullptr;
            }
        }
        break;
#endif
    }
    return nullptr;
}

}

extern "C" {

int minisketch_shahbits_supported(uint32_t shahbits) {
#ifdef ENABLE_FIELD_INT_2
    if (shahbits == 2) return true;
#endif
#ifdef ENABLE_FIELD_INT_3
    if (shahbits == 3) return true;
#endif
#ifdef ENABLE_FIELD_INT_4
    if (shahbits == 4) return true;
#endif
#ifdef ENABLE_FIELD_INT_5
    if (shahbits == 5) return true;
#endif
#ifdef ENABLE_FIELD_INT_6
    if (shahbits == 6) return true;
#endif
#ifdef ENABLE_FIELD_INT_7
    if (shahbits == 7) return true;
#endif
#ifdef ENABLE_FIELD_INT_8
    if (shahbits == 8) return true;
#endif
#ifdef ENABLE_FIELD_INT_9
    if (shahbits == 9) return true;
#endif
#ifdef ENABLE_FIELD_INT_10
    if (shahbits == 10) return true;
#endif
#ifdef ENABLE_FIELD_INT_11
    if (shahbits == 11) return true;
#endif
#ifdef ENABLE_FIELD_INT_12
    if (shahbits == 12) return true;
#endif
#ifdef ENABLE_FIELD_INT_13
    if (shahbits == 13) return true;
#endif
#ifdef ENABLE_FIELD_INT_14
    if (shahbits == 14) return true;
#endif
#ifdef ENABLE_FIELD_INT_15
    if (shahbits == 15) return true;
#endif
#ifdef ENABLE_FIELD_INT_16
    if (shahbits == 16) return true;
#endif
#ifdef ENABLE_FIELD_INT_17
    if (shahbits == 17) return true;
#endif
#ifdef ENABLE_FIELD_INT_18
    if (shahbits == 18) return true;
#endif
#ifdef ENABLE_FIELD_INT_19
    if (shahbits == 19) return true;
#endif
#ifdef ENABLE_FIELD_INT_20
    if (shahbits == 20) return true;
#endif
#ifdef ENABLE_FIELD_INT_21
    if (shahbits == 21) return true;
#endif
#ifdef ENABLE_FIELD_INT_22
    if (shahbits == 22) return true;
#endif
#ifdef ENABLE_FIELD_INT_23
    if (shahbits == 23) return true;
#endif
#ifdef ENABLE_FIELD_INT_24
    if (shahbits == 24) return true;
#endif
#ifdef ENABLE_FIELD_INT_25
    if (shahbits == 25) return true;
#endif
#ifdef ENABLE_FIELD_INT_26
    if (shahbits == 26) return true;
#endif
#ifdef ENABLE_FIELD_INT_27
    if (shahbits == 27) return true;
#endif
#ifdef ENABLE_FIELD_INT_28
    if (shahbits == 28) return true;
#endif
#ifdef ENABLE_FIELD_INT_29
    if (shahbits == 29) return true;
#endif
#ifdef ENABLE_FIELD_INT_30
    if (shahbits == 30) return true;
#endif
#ifdef ENABLE_FIELD_INT_31
    if (shahbits == 31) return true;
#endif
#ifdef ENABLE_FIELD_INT_32
    if (shahbits == 32) return true;
#endif
#ifdef ENABLE_FIELD_INT_33
    if (shahbits == 33) return true;
#endif
#ifdef ENABLE_FIELD_INT_34
    if (shahbits == 34) return true;
#endif
#ifdef ENABLE_FIELD_INT_35
    if (shahbits == 35) return true;
#endif
#ifdef ENABLE_FIELD_INT_36
    if (shahbits == 36) return true;
#endif
#ifdef ENABLE_FIELD_INT_37
    if (shahbits == 37) return true;
#endif
#ifdef ENABLE_FIELD_INT_38
    if (shahbits == 38) return true;
#endif
#ifdef ENABLE_FIELD_INT_39
    if (shahbits == 39) return true;
#endif
#ifdef ENABLE_FIELD_INT_40
    if (shahbits == 40) return true;
#endif
#ifdef ENABLE_FIELD_INT_41
    if (shahbits == 41) return true;
#endif
#ifdef ENABLE_FIELD_INT_42
    if (shahbits == 42) return true;
#endif
#ifdef ENABLE_FIELD_INT_43
    if (shahbits == 43) return true;
#endif
#ifdef ENABLE_FIELD_INT_44
    if (shahbits == 44) return true;
#endif
#ifdef ENABLE_FIELD_INT_45
    if (shahbits == 45) return true;
#endif
#ifdef ENABLE_FIELD_INT_46
    if (shahbits == 46) return true;
#endif
#ifdef ENABLE_FIELD_INT_47
    if (shahbits == 47) return true;
#endif
#ifdef ENABLE_FIELD_INT_48
    if (shahbits == 48) return true;
#endif
#ifdef ENABLE_FIELD_INT_49
    if (shahbits == 49) return true;
#endif
#ifdef ENABLE_FIELD_INT_50
    if (shahbits == 50) return true;
#endif
#ifdef ENABLE_FIELD_INT_51
    if (shahbits == 51) return true;
#endif
#ifdef ENABLE_FIELD_INT_52
    if (shahbits == 52) return true;
#endif
#ifdef ENABLE_FIELD_INT_53
    if (shahbits == 53) return true;
#endif
#ifdef ENABLE_FIELD_INT_54
    if (shahbits == 54) return true;
#endif
#ifdef ENABLE_FIELD_INT_55
    if (shahbits == 55) return true;
#endif
#ifdef ENABLE_FIELD_INT_56
    if (shahbits == 56) return true;
#endif
#ifdef ENABLE_FIELD_INT_57
    if (shahbits == 57) return true;
#endif
#ifdef ENABLE_FIELD_INT_58
    if (shahbits == 58) return true;
#endif
#ifdef ENABLE_FIELD_INT_59
    if (shahbits == 59) return true;
#endif
#ifdef ENABLE_FIELD_INT_60
    if (shahbits == 60) return true;
#endif
#ifdef ENABLE_FIELD_INT_61
    if (shahbits == 61) return true;
#endif
#ifdef ENABLE_FIELD_INT_62
    if (shahbits == 62) return true;
#endif
#ifdef ENABLE_FIELD_INT_63
    if (shahbits == 63) return true;
#endif
#ifdef ENABLE_FIELD_INT_64
    if (shahbits == 64) return true;
#endif
    return false;
}

uint32_t minisketch_implementation_max() {
    uint32_t ret = 0;
#ifdef HAVE_CLMUL
    ret += 2;
#endif
    return ret;
}

int minisketch_implementation_supported(uint32_t shahbits, uint32_t implementation) {
    if (!minisketch_shahbits_supported(shahbits) || implementation > minisketch_implementation_max()) {
        return 0;
    }
    try {
        Sketch* sketch = Construct(shahbits, implementation);
        if (sketch) {
            delete sketch;
            return 1;
        }
    } catch (const std::bad_alloc&) {}
    return 0;
}

minisketch* minisketch_create(uint32_t shahbits, uint32_t implementation, size_t capacity) {
    try {
        Sketch* sketch = Construct(shahbits, implementation);
        if (sketch) {
            try {
                sketch->Init(capacity);
            } catch (const std::bad_alloc&) {
                delete sketch;
                throw;
            }
            sketch->Ready();
        }
        return (minisketch*)sketch;
    } catch (const std::bad_alloc&) {
        return nullptr;
    }
}

uint32_t minisketch_shahbits(const minisketch* sketch) {
    const Sketch* s = (const Sketch*)sketch;
    s->Check();
    return s->shahbits();
}

size_t minisketch_capacity(const minisketch* sketch) {
    const Sketch* s = (const Sketch*)sketch;
    s->Check();
    return s->Syndromes();
}

uint32_t minisketch_implementation(const minisketch* sketch) {
    const Sketch* s = (const Sketch*)sketch;
    s->Check();
    return s->Implementation();
}

minisketch* minisketch_clone(const minisketch* sketch) {
    const Sketch* s = (const Sketch*)sketch;
    s->Check();
    Sketch* r = (Sketch*) minisketch_create(s->shahbits(), s->Implementation(), s->Syndromes());
    if (r) {
        r->Merge(s);
    }
    return (minisketch*) r;
}

void minisketch_destroy(minisketch* sketch) {
    if (sketch) {
        Sketch* s = (Sketch*)sketch;
        s->UnReady();
        delete s;
    }
}

size_t minisketch_serialized_size(const minisketch* sketch) {
    const Sketch* s = (const Sketch*)sketch;
    s->Check();
    size_t shahbits = s->shahbits();
    size_t syndromes = s->Syndromes();
    return (shahbits * syndromes + 7) / 8;
}

void minisketch_serialize(const minisketch* sketch, unsigned char* output) {
    const Sketch* s = (const Sketch*)sketch;
    s->Check();
    s->Serialize(output);
}

void minisketch_deserialize(minisketch* sketch, const unsigned char* input) {
    Sketch* s = (Sketch*)sketch;
    s->Check();
    s->Deserialize(input);
}

void minisketch_add_uint64(minisketch* sketch, uint64_t element) {
    Sketch* s = (Sketch*)sketch;
    s->Check();
    s->Add(element);
}

size_t minisketch_merge(minisketch* sketch, const minisketch* other_sketch) {
    Sketch* s1 = (Sketch*)sketch;
    const Sketch* s2 = (const Sketch*)other_sketch;
    s1->Check();
    s2->Check();
    if (s1->shahbits() != s2->shahbits()) return 0;
    if (s1->Implementation() != s2->Implementation()) return 0;
    return s1->Merge(s2);
}

ssize_t minisketch_decode(const minisketch* sketch, size_t max_elements, uint64_t* output) {
    const Sketch* s = (const Sketch*)sketch;
    s->Check();
    return s->Decode(max_elements, output);
}

void minisketch_set_seed(minisketch* sketch, uint64_t seed) {
    Sketch* s = (Sketch*)sketch;
    s->Check();
    s->SetSeed(seed);
}

size_t minisketch_compute_capacity(uint32_t shahbits, size_t max_elements, uint32_t fpshahbits) {
    return ComputeCapacity(shahbits, max_elements, fpshahbits);
}

size_t minisketch_compute_max_elements(uint32_t shahbits, size_t capacity, uint32_t fpshahbits) {
    return ComputeMaxElements(shahbits, capacity, fpshahbits);
}

}
