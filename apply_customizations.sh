######################################################################
# Additional integration and maintenance by:
# Copyright (C) 2025 The SHAHCOIN Core Developers — Hamid Rad
# https://shah.vip | info@shah.vip
# Distributed under the MIT License
######################################################################
nano apply_customizations.sh
#!/bin/bash

echo "🔧 Starting Shahcoin customization..."

# 1️⃣ Change project name from 'Shahcoin' to 'Shahcoin'
find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "configure.ac" \) -exec sed -i 's/Shahcoin/Shahcoin/g' {} +

# 2️⃣ Change symbol from 'SHAH' to 'SHAH'
find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "configure.ac" \) -exec sed -i 's/SHAH/SHAH/g' {} +

# 3️⃣ Change block subsidy from 50 to 20 coins
sed -i 's/static const CAmount nSubsidy = 50 \* COIN;/static const CAmount nSubsidy = 20 \* COIN;/g' src/validation.cpp

echo "✅ Customizations applied!"

