
const { EngineProcess } = require("./engine-process");

class Engine {
    constructor(name, path){
        this.name = name;
        this.path = path;
    }
    createProcess(handler){
        return new EngineProcess(this, handler);
    }
}

module.exports = { Engine };
