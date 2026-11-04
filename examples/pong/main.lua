-- Pong, in Lua. The same game as examples/pong-cpp, written against the `rivet` table instead of
-- subclassing Rivet::Application. `require` reads from the game folder, so these modules are
-- found the same way once the game ships inside an app.
local Ball = require("objects.ball")
local Paddle = require("objects.paddle")

local ball
local player
local opponent
local player_score = 0
local opponent_score = 0

function rivet.load()
    ball = Ball.new()
    player = Paddle.new("bottom")
    opponent = Paddle.new("top")
end

function rivet.update(delta)
    player:update_as_player(delta)
    opponent:update_as_ai(delta, ball)
    ball:update(delta)

    ball:bounce_off(player)
    ball:bounce_off(opponent)

    -- A ball that gets past a paddle is a point for the other side.
    if ball.y < 0.0 then
        player_score = player_score + 1
        ball:reset()
    elseif ball.y > rivet.graphics.get_height() then
        opponent_score = opponent_score + 1
        ball:reset()
    end

    if rivet.keyboard.is_pressed("escape") then
        rivet.quit()
    end
end

local function draw_score(score, center_y)
    local text = tostring(score)
    local size = 96.0
    local x = (rivet.graphics.get_width() - rivet.graphics.get_text_width(text, size)) / 2.0

    rivet.graphics.set_color(1.0, 1.0, 1.0, 0.15)
    rivet.graphics.print(text, x, center_y - size / 2.0, size)
end

function rivet.draw()
    rivet.graphics.clear(0.0, 0.0, 0.0)

    local width = rivet.graphics.get_width()
    local height = rivet.graphics.get_height()

    -- The halfway line, dashed so it reads as a court rather than a wall.
    rivet.graphics.set_color(1.0, 1.0, 1.0, 0.25)
    local dash = 24.0
    local x = 0.0
    while x < width do
        rivet.graphics.rectangle("fill", x, height / 2.0 - 1.0, dash, 2.0)
        x = x + dash * 2.0
    end

    draw_score(opponent_score, height * 0.25)
    draw_score(player_score, height * 0.75)

    ball:draw()
    player:draw()
    opponent:draw()
end
