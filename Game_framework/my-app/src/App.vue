<template>
  <v-app id="inspire">
    <v-card>
      <v-toolbar
        color="#845538"
        dark
        flat
        height="50px"
      >
        <template v-slot:extension>
          <div
            font-weight = "bold"
            v-text="text"
          >
          </div>
          <v-img
          width="55" 
          height="55"
          src="./assets/logo.svg"
          ></v-img>
          <v-tabs
            v-model="tab"
            align-with-title
          >
            <v-tabs-slider color="yellow"></v-tabs-slider>
            <v-tab
              v-for="item in items"
              :key="item"
            >
              {{ item }}
            </v-tab>
          </v-tabs>
        </template>
      </v-toolbar>
      <v-tabs-items v-model="tab">
        <v-tab-item>
          <v-card flat>
            <Klotski/>
          </v-card>
        </v-tab-item>
        <v-tab-item>
          <v-card flat>
            <FifteenPuzzle/>
          </v-card>
        </v-tab-item>
        <v-tab-item>
          <v-card flat>
            <Sokoban/>
          </v-card>
        </v-tab-item>
      </v-tabs-items>
    </v-card>
  </v-app>
</template>

<script>
import Klotski from './components/Klotski';
import FifteenPuzzle from './components/FifteenPuzzle';
import Sokoban from './components/Sokoban';

export default {
  name: 'App',

  components: {
    Klotski,
    FifteenPuzzle,
    Sokoban,
  },

  data: () => ({
      links: [
        'HOME',
        'PLAY',
        'BUILD',
        'SETTINGS',
        'HELP',
      ],
      items: [
        "Klotski",
        "15-puzzle",
        "Sokoban",
      ],
      tab: [
        "L",
        "C",
        "H",
      ],
      text: "Sliding Puzzles",
    }),
};
</script>
