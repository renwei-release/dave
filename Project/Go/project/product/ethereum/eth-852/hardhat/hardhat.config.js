require("@nomicfoundation/hardhat-toolbox");

/** @type import('hardhat/config').HardhatUserConfig */
module.exports = {
  solidity: "0.8.24",
  networks: {
    hk852network : {
      url: "http://47.238.205.52:8545",
      accounts: ["5cc85bbc74b347616492ce6bc4c20348211a6b791807f093c465c313809e8ff4"],
    },
  },
};
