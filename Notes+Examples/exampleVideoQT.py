import signal
import sys


from PySide2.QtCore import QUrl
from PySide2.QtMultimedia import QMediaPlayer, QMediaPlaylist, QMediaContent
from PySide2.QtMultimediaWidgets import QVideoWidget, QGraphicsVideoItem
from PySide2.QtWidgets import QGraphicsScene, QGraphicsView, QApplication

PATH='/home/chittohthesupperior/Pictures/reagan.mp4'


if __name__ == '__main__':
	app = QApplication(sys.argv)
	scene = QGraphicsScene()
	view = QGraphicsView()
	view.setScene(scene)
	view.show()
	media_player = QMediaPlayer()
	video_widget = QGraphicsVideoItem()
	current_play_list = QMediaPlaylist()
	media_player.setVideoOutput(video_widget)
	media_player.setPlaylist(current_play_list)
	video_proxy = scene.addItem(video_widget)
	video_widget.show()
	current_play_list.addMedia(QMediaContent(QUrl.fromLocalFile(PATH)))
	media_player.play()
	signal.signal(signal.SIGINT, signal.SIG_DFL)
	app.exec_()
