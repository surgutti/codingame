for bot in bot_19-15 bot_10-15 bot_7-15 bot_13-15 bot_16-15 bot_4-15 bot_1-15 bot_19-8 bot_16-8 bot_13-8 bot_10-8 bot_7-8 bot_4-8; do
	echo "stoping $bot"
	psyleague bot stop $bot
done
