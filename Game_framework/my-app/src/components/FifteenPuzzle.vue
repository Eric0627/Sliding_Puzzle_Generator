<template>
  <div class = "gameboard">
    <div class="step"><br>{{moves}}</div>
    <input class="back" type="button" v-on:click="moveBack">
    <input class="restart" type="button" v-on:click="startNewGame"> 
    <input id="lastLevel2" type="button" v-on:click="changeLevel($event)">
    <div id="level"><br>{{level+1}}</div>
    <input id="nextLevel2" type="button" v-on:click="changeLevel($event)">
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
            <span class="headline">Cusotmize 15-puzzles</span>
          </v-card-title>
          <v-card-text>
            <v-container>
              <v-row>
                <v-col
                  cols="12"
                  sm="6"
                  md="4"
                >
                  <v-text-field
                    label="Maximum level (integer)"
                    v-model="maxLev"
                  ></v-text-field>
                </v-col>
                <v-col
                  cols="12"
                  sm="6"
                >
                  <v-text-field
                    v-model="numOfPuzzles"
                    label="Number of puzzles (integer)"
                    required
                  ></v-text-field>
                </v-col>
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
              @click="newPuzzles($event)"
            >
              Submit
            </v-btn>
          </v-card-actions>
        </v-card>
      </v-dialog>
    </v-row>
		<div class="kuang2" v-bind:style="{backgroundSize: bgSize}">
			<div id="puzzle" style="position:relative;left:20px;top:20px;"></div>
		</div>
  </div>
</template>

<script>
import { generatePuzzles } from '../plugins/UserService'
  export default {
    name: 'FifteenPuzzle',
    data () {
      return {
        moves: 0,
// MAP:[[[8,6,7,],
// [2,5,4,],
// [3,0,1,]],
// [[6,4,7,],
// [8,5,0,],
// [3,2,1,]],
// ],
//MAP:[[[9,5,7,3],[13,1,11,4],[10,0,15,8],[6,2,14,12]],[[9,5,7,3],[13,1,11,4],[10,2,15,8],[6,14,0,12]],[[9,0,7,3],[13,5,11,4],[2,1,15,8],[10,6,14,12]],[[9,5,7,3],[0,13,11,4],[2,1,15,8],[10,6,14,12]],[[9,5,7,3],[13,11,0,4],[2,1,15,8],[10,6,14,12]],[[9,5,7,3],[13,1,11,4],[2,6,15,8],[0,10,14,12]],[[9,5,7,3],[13,1,11,4],[2,6,15,8],[10,14,0,12]],[[9,5,7,3],[13,1,0,4],[2,15,11,8],[10,6,14,12]],[[9,5,7,3],[13,1,11,4],[2,15,14,8],[10,6,0,12]],[[9,5,7,3],[13,1,11,4],[2,15,8,0],[10,6,14,12]],[[9,7,11,3],[1,5,0,4],[13,2,15,8],[10,6,14,12]],[[9,7,3,0],[1,5,11,4],[13,2,15,8],[10,6,14,12]],[[9,5,7,3],[1,2,11,4],[13,6,15,8],[10,14,0,12]],[[9,5,7,3],[0,2,11,4],[1,13,15,8],[10,6,14,12]],[[9,5,7,3],[1,2,11,4],[10,13,15,8],[0,6,14,12]],[[9,5,7,3],[1,2,0,4],[13,15,11,8],[10,6,14,12]],[[9,5,7,3],[1,2,11,4],[13,15,14,8],[10,6,0,12]],[[9,5,7,3],[1,2,11,4],[13,15,8,0],[10,6,14,12]],[[9,0,5,3],[1,11,7,4],[13,2,15,8],[10,6,14,12]],[[9,5,7,3],[1,11,15,4],[13,2,14,8],[10,6,0,12]]],
//sol:[[[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[1,1],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[1,1],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[1,1],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[3,1],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[3,1],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,2],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,2],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,2],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[0,2],[0,1],[1,1],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[0,2],[0,1],[1,1],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[3,1],[2,1],[1,1],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,0],[2,1],[1,1],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,0],[2,1],[1,1],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,2],[2,1],[1,1],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,2],[2,1],[1,1],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,2],[2,1],[1,1],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[0,2],[1,2],[1,1],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,2],[1,2],[1,1],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]]],
MAP:[[[9,0,7,3],[13,5,11,4],[10,1,15,8],[6,2,14,12]],[[9,5,7,3],[0,13,11,4],[10,1,15,8],[6,2,14,12]],[[9,5,7,3],[13,11,0,4],[10,1,15,8],[6,2,14,12]],[[9,5,7,3],[0,1,11,4],[13,10,15,8],[6,2,14,12]],[[9,5,7,3],[13,1,11,4],[6,10,15,8],[0,2,14,12]],[[9,5,7,3],[13,1,0,4],[10,15,11,8],[6,2,14,12]],[[9,5,7,3],[13,1,11,4],[10,15,14,8],[6,2,0,12]],[[9,5,7,3],[13,1,11,4],[10,15,8,0],[6,2,14,12]],[[9,5,7,3],[13,1,11,4],[10,0,2,8],[6,14,15,12]],[[9,5,7,3],[13,1,11,4],[10,2,8,0],[6,14,15,12]],[[9,5,7,3],[13,1,11,4],[10,2,15,0],[6,14,12,8]],[[13,9,7,3],[0,5,11,4],[2,1,15,8],[10,6,14,12]],[[9,7,11,3],[13,5,0,4],[2,1,15,8],[10,6,14,12]],[[9,7,3,0],[13,5,11,4],[2,1,15,8],[10,6,14,12]],[[5,0,7,3],[9,13,11,4],[2,1,15,8],[10,6,14,12]],[[9,5,7,3],[2,13,11,4],[10,1,15,8],[0,6,14,12]],[[9,5,7,3],[2,13,11,4],[1,0,15,8],[10,6,14,12]],[[9,0,5,3],[13,11,7,4],[2,1,15,8],[10,6,14,12]],[[9,5,7,3],[13,11,15,4],[2,1,14,8],[10,6,0,12]],[[9,5,7,3],[13,11,15,4],[2,0,1,8],[10,6,14,12]]],
sol:[[[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,0],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,0],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,2],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,2],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,2],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,2],[3,2],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,2],[3,2],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[3,3],[3,2],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[0,0],[0,1],[1,1],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[0,2],[0,1],[1,1],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[0,2],[0,1],[1,1],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[0,0],[1,0],[1,1],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,0],[1,0],[1,1],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,0],[1,0],[1,1],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[0,2],[1,2],[1,1],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,2],[1,2],[1,1],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]],[[2,2],[1,2],[1,1],[2,1],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,0],[2,0],[1,0],[0,0],[0,1],[1,1],[2,1],[3,1],[3,2],[2,2],[1,2],[0,2],[0,3],[1,3],[2,3],[3,3]]],

        level: 0,
        rows: 0,
        cols: 0,
        bgSize: "0px 0px",
        moveInfo: [],
        arrayForBoard: new Array(this.rows),
        neighbors: [[-1,0],[1,0],[0,-1],[0,1]],//up, down, left, right

        dialog: false,
        maxLev: '',
        numOfPuzzles:'',
      }
    },
    mounted: function(){
        this.startNewGame();
    },
    methods: {
      newPuzzles(data) {
        // console.log('data:::', data)
        // console.log("Max level", this.maxLev, this.type)
        if(this.numOfPuzzles==='') {
          this.dialog = false;
          return;
        }
        generatePuzzles(["15puzzle.exe", this.maxLev, this.numOfPuzzles]).then(response => {
          console.log(response);
          this.MAP = response["MAP"];
          this.sol = response["sol"];
          this.level = 0, this.steps = 0;
          this.dialog = false;
          let last=document.getElementById("lastLevel2"),
              next=document.getElementById("nextLevel2");
          last.style.visibility="hidden";
          next.style.visibility="visible";
          this.startNewGame();
        });
      },
    startNewGame(){
      var map = this.MAP[this.level];
      this.moves = 0, this.moveInfo = [];
      this.rows = map.length;
      if(this.rows>0) this.cols = map[0].length;
      //set background size
      this.bgSize = this.cols*50+50+"px "+(this.rows*50+50)+"px";
      // Create the proper board size.
      for(let i = 0; i < this.rows; i++) this.arrayForBoard[i]=new Array(this.cols);

      // Assign numbers to the game board.
      for(let i = 0; i < this.rows; i++){
          for(let j = 0; j < this.cols; j++){
          this.arrayForBoard[i][j] = map[i][j];

          }
      }
      this.showTable();
    },
    changeLevel(e){
      if(e.currentTarget.id=="nextLevel2") this.level++;
      else this.level--;
      if(this.level==this.MAP.length-1||this.level==0) {
        e.currentTarget.style.visibility="hidden";
        if(this.MAP.length==2){//special case: only two maps
          let last=document.getElementById("lastLevel2"),
              next=document.getElementById("nextLevel2");
          if(this.level==1) last.style.visibility="visible";
          else next.style.visibility="visible";
        }
        this.level = this.level%this.MAP.length;
      }
      else {
        var last=document.getElementById("lastLevel2"),
            next=document.getElementById("nextLevel2");
        last.style.visibility="visible";
        next.style.visibility="visible";
      }
      this.startNewGame();
    },
    test(){
        console.log("hahaha");
    },
    autoPlay(){
      this.startNewGame();
      var sol = this.sol[this.level];
      for(let i = 0; i < sol.length; i++){
        setTimeout(() => {
          console.log(i);
          this.moveThisTile(sol[i][0],sol[i][1]);
        }, 200 * i);
      }
    },
    moveBack(){
      if(this.moveInfo.length!=0){
        var lastTile = this.moveInfo.pop();
        this.moveThisTile(lastTile[0], lastTile[1]);
        this.moves-=2;//reverse move data
        this.moveInfo.pop();
      }
    },
    showTable(){
        var outputString = "<table id = \"newtable\" border = 1>";
        var table = document.createElement('table');
        table.id = "newTable";
        table.style.border = 1;
        for(var i = 0; i < this.rows; i++){
            outputString += "<tr>";
            let tr = document.createElement('tr');
            for(var j = 0; j < this.cols; j++){
                let td = document.createElement('td');
                if(this.arrayForBoard[i][j] == 0) td.className = "blank";
                else {
                    outputString += "<td class=\"tile\" v-on:click=\"test\">" + this.arrayForBoard[i][j] + "</td>";
                    td.className = "tile";
                    td.innerHTML = this.arrayForBoard[i][j];
                    td.addEventListener("click", this.moveThisTile.bind(this,i,j));
                }
                tr.appendChild(td);
            } // end for (var j = 0; j < columns; j++)
            outputString += "</tr>";
            table.appendChild(tr);
        } // end for (var i = 0; i < rows; i++)
        outputString += "</table>";
        var game=document.getElementById("puzzle");
        //game.innerHTML = outputString;
        if(game.lastChild!=null) game.removeChild(game.lastChild);
        game.appendChild(table);
        },
    moveThisTile(row, col){
        for(let i=0;i<4;i++){
            var nextR=row+this.neighbors[i][0], nextC=col+this.neighbors[i][1];
            if (nextR >= 0 && nextC >= 0 && nextR < this.rows && nextC < this.cols){
                if (this.arrayForBoard[nextR][nextC] == 0){
                this.arrayForBoard[nextR][nextC] = this.arrayForBoard[row][col];
                this.arrayForBoard[row][col] = 0;
                this.moves++;
                this.moveInfo.push([nextR,nextC]);
                this.showTable();
                }
            }
        }        
        if (this.checkIfWinner())
        {
          setTimeout(() => {
              alert("Congratulations! You solved the puzzle in " + this.moves + " moves.");
            }, 100);
          // this.startNewGame();
        }
        return false; 
    },
    checkIfWinner(){
        var count = 1;
        for(var i = 0; i < this.rows; i++){
            for(var j = 0; j < this.cols; j++){
                if(this.arrayForBoard[i][j] != count){
                    if(!(count === this.rows * this.cols && this.arrayForBoard[i][j] === 0 )) 
                        return false;
                }
                count++;
            }
        }
        return true;
        },
    }
  }

</script>

<style>
  #table{
    width: auto;
    margin-left: auto;
    margin-right: auto;
    padding: 10px;
  }

  .tile {
    width: 50px;
    height: 50px;
    /* border: 2px solid #222; */
    text-align: center;
    font-size: 18px;
    font-weight: bold;
    /* Webkit (Chrome and Safari) */
    /* background: -webkit-gradient(linear, top, bottom,color-stop(15%, white),color-stop(80%, darkgray),); */
    /* Mozilla */
    /* background: -moz-linear-gradient(top, white 15%, darkgray 90%);   */
    /* Internet Explorer */
    /* background: linear-gradient(90deg, white 15%, darkgray 90%); */
    background: url(../assets/blank.png) no-repeat;
    /* background-position: -4px -10px; */
    color: #000;
    cursor: pointer;
  }

  .blank {
    width: 50px;
    height: 50px;
    text-align: center;
  }
  .kuang2 {
    left: 40%; top: 10%;
    position: relative;
    width: 276px; height: 333px;
    background: url(../assets/kuang.png) no-repeat;
    /* background-size: 200px 200px; */
  }
  #lastLevel2{
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
  #nextLevel2{
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
