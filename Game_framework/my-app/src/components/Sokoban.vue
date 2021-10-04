<template>
  <div class = "gameboard">
    <div class="step"><br>{{moves}}</div>
    <input class="back" type="button" v-on:click="moveBack">
    <input class="restart" type="button" v-on:click="startNewGame">
    <input id="lastLevel3" type="button" v-on:click="changeLevel($event)">
    <div id="level"><br>{{level+1}}</div>
    <input id="nextLevel3" type="button" v-on:click="changeLevel($event)">
    <input class="solution" type="button" v-on:click="autoPlay">
    <v-row justify="center">
      <v-dialog
        v-model="dialog"
        persistent
        max-width="600px"
      >
        <template v-slot:activator="{ on, attrs }">
          <input class="setting" type="button" 
            v-bind="attrs"
            v-on="on"
          >
        </template>
                <v-card>
          <v-card-title>
            <span class="headline">Cusotmize Sokoban puzzles</span>
          </v-card-title>
          <v-card-text>
            <v-container>
              <v-row>
                <input type="file" @change="onFileChange" />
                <span>* Upload an image or text file</span>
                <v-text-field
                    v-model="numOfRows"
                    label="Number of rows (for image)"
                ></v-text-field>
                <v-text-field
                    v-model="numOfCols"
                    label="Number of columns (for image)"
                ></v-text-field>
            </v-row>
            </v-container>
            <small>* indicates required field</small>
          </v-card-text>
          <v-card-actions>
            <v-spacer></v-spacer>
            <v-btn
              color="blue darken-1"
              text
              @click="dialog = false"
            >
              Close
            </v-btn>
            <v-btn
              color="blue darken-1"
              text
              @click="onUploadFile"
            :disabled="!this.selectedFile"
            >
              Submit
            </v-btn>
          </v-card-actions>
        </v-card>
        <!-- <div class="file-upload">
            
        </div> -->
      </v-dialog>
    </v-row>
    <div class="kuang3" v-bind:style="{backgroundSize: bgSize}">
        <div id="sokoban" style="position:relative;left:20px;top:20px;"></div>
    </div>
  </div>
</template>

<script>
import axios from "axios";

  export default {
    name: 'FifteenPuzzle',
    data () {
      return {
        moves: 0,
        level: 0,
		MAP:[[[0,0,0,5,5,5,5,5,5,0],[0,5,5,5,3,3,3,3,5,0],[5,5,4,3,3,5,5,2,5,5],[5,4,4,3,3,3,3,3,4,5],[5,1,2,3,2,3,2,3,5,5],[5,5,5,5,5,5,3,3,5,0],[0,0,0,0,0,5,5,5,5,0]],[[0,0,0,5,5,5,5,5,5,0],[0,5,5,5,3,3,3,3,5,0],[5,5,4,3,3,5,5,2,5,5],[5,4,4,3,3,3,3,3,4,5],[5,3,2,3,2,1,2,3,5,5],[5,5,5,5,5,5,3,3,5,0],[0,0,0,0,0,5,5,5,5,0]]],
        sol:[[[4,1,0],[3,1,3],[3,2,3],[3,3,1],[4,3,3],[4,4,0],[3,4,3],[3,5,3],[3,6,3],[3,7,1],[4,7,1],[5,7,2],[5,6,0],[4,6,3],[4,7,0],[3,7,2],[3,6,2],[3,5,2],[3,4,1],[4,4,3],[4,5,2],[4,4,2],[4,3,0],[3,3,2],[3,2,2],[3,1,1],[4,1,3],[4,2,3],[4,3,3],[4,4,0],[3,4,0],[2,4,2],[2,3,3],[2,4,0],[1,4,3],[1,5,3],[1,6,3],[1,7,1],[2,7,0],[1,7,2],[1,6,2],[1,5,2],[1,4,1],[2,4,1],[3,4,3],[3,5,3],[3,6,3],[3,7,1],[4,7,1],[5,7,2],[5,6,0],[4,6,3],[4,7,0],[3,7,2],[3,6,2],[3,5,2],[3,4,2],[3,3,2],[3,2,3],[3,3,3],[3,4,1],[4,4,3],[4,5,0],[3,5,3],[3,6,3],[3,7,1],[4,7,1],[5,7,2],[5,6,0],[4,6,3],[4,7,0],[3,7,2],[3,6,2],[3,5,2],[3,4,2]],[[4,5,0],[3,5,2],[3,4,2],[3,3,1],[4,3,3],[4,4,0],[3,4,3],[3,5,3],[3,6,3],[3,7,1],[4,7,1],[5,7,2],[5,6,0],[4,6,3],[4,7,0],[3,7,2],[3,6,2],[3,5,2],[3,4,1],[4,4,3],[4,5,2],[4,4,2],[4,3,0],[3,3,2],[3,2,2],[3,1,1],[4,1,3],[4,2,3],[4,3,3],[4,4,0],[3,4,0],[2,4,2],[2,3,3],[2,4,0],[1,4,3],[1,5,3],[1,6,3],[1,7,1],[2,7,0],[1,7,2],[1,6,2],[1,5,2],[1,4,1],[2,4,1],[3,4,3],[3,5,3],[3,6,3],[3,7,1],[4,7,1],[5,7,2],[5,6,0],[4,6,3],[4,7,0],[3,7,2],[3,6,2],[3,5,2],[3,4,2],[3,3,2],[3,2,3],[3,3,3],[3,4,1],[4,4,3],[4,5,0],[3,5,3],[3,6,3],[3,7,1],[4,7,1],[5,7,2],[5,6,0],[4,6,3],[4,7,0],[3,7,2],[3,6,2],[3,5,2],[3,4,2]]],
        rows: 0,
        cols: 0,
        bgSize: "0px 0px",
        moveInfo: [],
        neighbors: [[-1,0],[1,0],[0,-1],[0,1]],//up, down, left, right

        dialog: false,
        selectedFile: "",
        numOfRows: 0,
        numOfCols: 0,
      }
    },
    mounted: function(){
        this.startNewGame();
    },
    methods: {
        onFileChange(e) {
            const selectedFile = e.target.files[0]; // accessing file
            this.selectedFile = selectedFile;
        },
        onUploadFile() {
            const formData = new FormData();
            formData.append("file", this.selectedFile);  // appending file
            formData.append("rows", this.numOfRows);
            formData.append("cols", this.numOfCols);
            console.log("image size", this.numOfRows, this.numOfCols);
            // sending file to the backend
            axios.post("http://localhost:3080/api/upload", formData)
                .then(res => {
                console.log(res.data);
                this.MAP = res.data["MAP"];
                this.sol = res.data["sol"];
                this.level = 0, this.steps = 0;
                this.dialog = false;
                let last=document.getElementById("lastLevel3"),
                    next=document.getElementById("nextLevel3");
                last.style.visibility="hidden";
                next.style.visibility="visible";
                this.startNewGame();
                })
                .catch(err => {
                console.log(err);
                });
        },
        startNewGame() {
            var map = this.MAP[this.level];
            this.moves = 0, this.moveInfo = [];
            this.rows = map.length;
            if(this.rows>0) this.cols = map[0].length;
            this.bgSize = this.cols*40+50+"px "+(this.rows*40+50)+"px";
            //construct gameboard
            var html = '<table>';
            for(let i = 0; i < this.rows; i++) {
                html += '<tr>';
                for(let j = 0; j < this.cols; j++) {
                    html += '<td id=' +  i + '_' + j + '></td>';
                }
                html += '</tr>';
            }
            html += '</table>';
            document.getElementById('sokoban').innerHTML = html;
            //set class name for each element
            var elem = {
                1: "man",
                2: "box", //box
                3: "ground",
                4: "target",
                5: "wall",
                6: "arrive",
            };
            for(let i = 0; i < this.rows; i++){
                for(let j = 0; j < this.cols; j++){
                    document.getElementById(i + '_' + j).className = elem[map[i][j]];
                    document.getElementById(i + '_' + j).dataset.class = elem[map[i][j]];
                    if(map[i][j]==6) document.getElementById(i + '_' + j).dataset.class = "target";
                }
            }
            this.keyListen();
        },
        keyListen(){//listen to the key event
            var move=this.move;
            var neighbors=this.neighbors;
            var rows=this.rows, cols=this.cols;
            document.onkeydown = function(event) {//find the direction
                var playerPos = document.querySelector('.man').id.split('_');
                playerPos[0]=parseInt(playerPos[0]),
                playerPos[1]=parseInt(playerPos[1]);//convert string into integer
                var dir=-1;
                switch(event.keyCode){
                    case 37: dir=2;break;//left
                    case 38: dir=0;break;//up
                    case 39: dir=3;break;//right
                    case 40: dir=1;break;//down
                }
                if(dir != -1){
                    var row = playerPos[0]+neighbors[dir][0],
                        col = playerPos[1]+neighbors[dir][1];
                    //check if next pos is out of boundary
                    if(row<0||col<0||row>=rows||col>=cols) return;
                    //check if next pos is wall
                    if (document.getElementById(row + '_' + col).className == 'wall') return;
                    else if (document.getElementById(row + '_' + col).className == 'box' //if next pos is box
                        || document.getElementById(row + '_' + col).className == 'arrive') {
                        row = row+neighbors[dir][0],
                        col = col+neighbors[dir][1];
                        if(row<0||col<0||row>=rows||col>=cols) return;
                        if (document.getElementById(row + '_' + col).className == 'wall' 
                        || document.getElementById(row + '_' + col).className == 'arrive' 
                        || document.getElementById(row + '_' + col).className == 'box') {
                            return;
                        }
                        row = row-neighbors[dir][0],
                        col = col-neighbors[dir][1];
                    }
                    move(playerPos, dir);//move the player
                }
            }
        },
        move(cur, dir){
            var row = cur[0]+this.neighbors[dir][0],
                col = cur[1]+this.neighbors[dir][1];
            var next = [row, col];
            var isBoxMoved = false;
            //move the player
            //change the current pos into target or ground
            if (document.getElementById(cur[0] + '_' + cur[1]).dataset.class == 'target') {
                document.getElementById(cur[0] + '_' + cur[1]).className = 'target';
            } else {
                document.getElementById(cur[0] + '_' + cur[1]).className = 'ground';
            }
            //check if the next pos is ground or target
            if (document.getElementById(next[0] + '_' + next[1]).className == 'ground' 
                || document.getElementById(next[0] + '_' + next[1]).className == 'target') {
                document.getElementById(next[0] + '_' + next[1]).className = 'man';
            } 
            else {//next pos is box
                row = row+this.neighbors[dir][0],
                col = col+this.neighbors[dir][1];
                isBoxMoved = true;
            }
            //check whether the next next pos is ground or target
            if (document.getElementById(row + '_' + col).className == 'ground') {
                document.getElementById(row + '_' + col).className = 'box';
                document.getElementById(next[0] + '_' + next[1]).className = 'man';
            } 
            else {
                document.getElementById(row + '_' + col).className = 'arrive';
                document.getElementById(next[0] + '_' + next[1]).className = 'man';
            }
            this.moves++;
            this.moveInfo.push([next, dir, isBoxMoved]);
            this.isWin();
        },
        moveBack(){
            if(this.moves==0 || this.moveInfo.length==0) return;
            var lastInfo = this.moveInfo.pop();
            var cur = lastInfo[0], dir = lastInfo[1], isBoxMoved = lastInfo[2];
            switch(dir){
                case 0: dir=1;break;
                case 1: dir=0;break;
                case 2: dir=3;break;
                case 3: dir=2;break;
            }
            var row = cur[0]+this.neighbors[dir][0],
                col = cur[1]+this.neighbors[dir][1];
            var next = [row, col];
            //move the player
            document.getElementById(next[0] + '_' + next[1]).className = 'man';
            //change the current pos into target or ground
            if (document.getElementById(cur[0] + '_' + cur[1]).dataset.class == 'target') {
                document.getElementById(cur[0] + '_' + cur[1]).className = 'target';
            } else {
                document.getElementById(cur[0] + '_' + cur[1]).className = 'ground';
            }
            if(isBoxMoved){
                row = cur[0]-this.neighbors[dir][0],
                col = cur[1]-this.neighbors[dir][1];
                if(document.getElementById(row + '_' + col).className == 'box') {
                    if(document.getElementById(cur[0] + '_' + cur[1]).className == 'target')
                        document.getElementById(cur[0] + '_' + cur[1]).className = 'arrive';
                    else document.getElementById(cur[0] + '_' + cur[1]).className = 'box';
                    document.getElementById(row + '_' + col).className = 'ground'
                }
                else if(document.getElementById(row + '_' + col).className == 'arrive') {
                    if(document.getElementById(cur[0] + '_' + cur[1]).className == 'target')
                        document.getElementById(cur[0] + '_' + cur[1]).className = 'arrive';
                    else document.getElementById(cur[0] + '_' + cur[1]).className = 'box';
                    document.getElementById(row + '_' + col).className = 'target'
                }
            }
            this.moves--;
        },
        isWin(){
            if (!document.querySelector('.box')) {
                setTimeout(() => {
                    alert("Congratulations! You solved the puzzle in " + this.moves + " moves.");
                }, 100);
            }
        },
        autoPlay(){
            this.startNewGame();
            var sol = this.sol[this.level];
            for(let i = 0; i < sol.length; i++){
                setTimeout(() => {
                console.log(i);
                this.move([sol[i][0],sol[i][1]],sol[i][2]);
                }, 200 * i);
            }
        },
        changeLevel(e){
            if(e.currentTarget.id=="nextLevel3") this.level++;
            else this.level--;
            if(this.level==this.MAP.length-1||this.level==0) {
                e.currentTarget.style.visibility="hidden";
                if(this.MAP.length==2){//special case: only two maps
                let last=document.getElementById("lastLevel3"),
                    next=document.getElementById("nextLevel3");
                if(this.level==1) last.style.visibility="visible";
                else next.style.visibility="visible";
                }
                this.level = this.level%this.MAP.length;
            }
            else {
                var last=document.getElementById("lastLevel3"),
                    next=document.getElementById("nextLevel3");
                last.style.visibility="visible";
                next.style.visibility="visible";
            }
            this.startNewGame();
        },
    }
  }

</script>

<style>
    .kuang3 {
        left: 40%; top: 10%;
        position: relative;
        width: auto; height: 100%;
        background: url(../assets/kuang.png) no-repeat;
        /* background-size: 200px 200px; */
    }
    #sokoban table {
        border-collapse: collapse;
        margin-left: 5px;
    }
    #sokoban td {
        width: 40px;
        height: 40px;
        /* background-color: #000; */
    }
    .wall {
        background: url(../assets/Sokoban/wall.png);
        background-size: 40px 40px;
    }

    .ground {
        background: url(../assets/Sokoban/ground.png);
        background-size: 40px 40px;
    }

    .man {
        background: url(../assets/Sokoban/man.png);
        background-size: 40px 40px;
    }

    .box {
        background: url(../assets/Sokoban/box.png);
        background-size: 40px 40px;
    }

    .target {
        background: url(../assets/Sokoban/target.png);
        background-size: 40px 40px;
    }

    .arrive {
        background: url(../assets/Sokoban/arrive.png);
        background-size: 40px 40px;
    }
    .prompt {
        float: left;
    }
    #lastLevel3{
    position: absolute;
    left: 21%;
    top: 10%;
    width: 22px; height: 44px;
    background-image:url(../assets/arrow.png);
    background-position: 0px 0px;
    z-index:1;
    visibility: hidden;
  }
  #level {
    position: absolute;
    left: 20%;
    top: 5%;
    width: 108px; height: 75px;
    font:bold, "sans-serif", 20px;
    color: white;
    text-align: center;
    background-image:url(../assets/level.png);
  }
  #nextLevel3{
    position: absolute;
    left: 32%;
    top: 10%;
    width: 22px; height: 44px;
    background-image:url(../assets/arrow.png);
    background-position: -22px 0px;
    z-index:1;
    visibility: visible;
  }
</style>
