local Paddle = {}
Paddle.__index = Paddle

local WIDTH = 160.0
local HEIGHT = 24.0
local MARGIN = 96.0
local SPEED = 900.0

-- `side` is "bottom" for the player's paddle or "top" for the opponent's.
function Paddle.new(side)
    local self = setmetatable({}, Paddle)

    self.width = WIDTH
    self.height = HEIGHT
    self.x = (rivet.graphics.get_width() - WIDTH) / 2.0

    if side == "top" then
        self.y = MARGIN
    else
        self.y = rivet.graphics.get_height() - MARGIN - HEIGHT
    end

    return self
end

function Paddle:center_on(x, delta)
    local target = x - self.width / 2.0
    local step = SPEED * delta

    -- Moved towards the target rather than onto it, so the paddle can be outrun.
    if target > self.x then
        self.x = math.min(self.x + step, target)
    else
        self.x = math.max(self.x - step, target)
    end

    self:clamp()
end

function Paddle:clamp()
    self.x = math.max(0.0, math.min(self.x, rivet.graphics.get_width() - self.width))
end

-- The player's paddle follows a touch or the mouse, and the arrow keys move it on a desktop.
function Paddle:update_as_player(delta)
    if rivet.touch.get_count() > 0 then
        local x = rivet.touch.get_position(1)
        self:center_on(x, delta)
        return
    end

    if rivet.mouse.is_down("left") then
        local x = rivet.mouse.get_position()
        self:center_on(x, delta)
        return
    end

    if rivet.keyboard.is_down("left") or rivet.keyboard.is_down("a") then
        self.x = self.x - SPEED * delta
    end
    if rivet.keyboard.is_down("right") or rivet.keyboard.is_down("d") then
        self.x = self.x + SPEED * delta
    end

    self:clamp()
end

-- The opponent follows the ball, but only once it's coming towards them, so it can be beaten.
function Paddle:update_as_ai(delta, ball)
    if ball.velocity_y < 0.0 then
        self:center_on(ball.x, delta)
    end
end

function Paddle:draw()
    rivet.graphics.set_color(1.0, 1.0, 1.0)
    rivet.graphics.rectangle("fill", self.x, self.y, self.width, self.height)
end

return Paddle
