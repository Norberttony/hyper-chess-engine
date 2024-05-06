
const fs = require("fs");

// returns all positions currently stored in positions.txt
// removes any duplicates in the positions.txt file
function getAllPositions(){
    let positions = fs.readFileSync("./positions.txt").toString().split("\n");

    // remove duplicates
    let duplicatesRemoved = 0;
    for (let i = 0; i < positions.length; i++){
        let index = positions.indexOf(positions[i], i + 1);
        while (index > -1){
            positions.splice(index, 1);
            duplicatesRemoved++;
            index = positions.indexOf(positions[i], index);
        }
    }
    console.log(`Removed ${duplicatesRemoved} duplicates`);
    console.log(`Left with ${positions.length} positions`);

    return positions;
}

module.exports = { getAllPositions };
