module.exports = {
  devServer: {
    proxy: {
      '^/api': {
        target: 'http://localhost:3080',
        changeOrigin: true
      },
    }
  },
  "transpileDependencies": [
    "vuetify"
  ],
  runtimeCompiler: true,
  publicPath: './',
  pluginOptions: {
    electronBuilder: {
      nodeIntegration: true
    }
  },
  // configureWebpack: (config) => {
  //   config.externals = { 'child_process': 'require("electron").remote.require("child_process")' };
  // },
}